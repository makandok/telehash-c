#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "e3x.h"
#include "util.h"

// load secrets/keys to create a new local endpoint
e3x_self_t e3x_self_new(lob_t secrets, lob_t keys)
{
  uint8_t i, csids = 0, hash[32];
  e3x_self_t self;
  if(!keys) keys = lob_linked(secrets); // convenience
  if(!keys) return NULL;

  if(!(self = malloc(sizeof (struct e3x_self_struct)))) return NULL;
  memset(self,0,sizeof (struct e3x_self_struct));

  // give each cset a chance to make a local
  for(i=0; i<CS_MAX; i++)
  {
    if(!e3x_cipher_sets[i] || !e3x_cipher_sets[i]->local_new) continue;
    self->locals[i] = e3x_cipher_sets[i]->local_new(keys, secrets);
    if(!self->locals[i]) continue;
    // make a copy of the binary and encoded keys
    self->keys[i] = lob_get_base32(keys, e3x_cipher_sets[i]->hex);
    lob_set(self->keys[i],"key",lob_get(keys,e3x_cipher_sets[i]->hex));
    // make a hash for the intermediate form for hashnames
    e3x_hash(self->keys[i]->body,self->keys[i]->body_len,hash);
    lob_set_base32(self->keys[i],"hash",hash,32);
    csids++;
  }

  if(!csids)
  {
    e3x_self_free(self);
    return LOG("self failed for %.*s",keys->head_len,keys->head);
  }

  LOG("self created with %d csids",csids);
  return self;
}

// any exchanges must have been free'd first
void e3x_self_free(e3x_self_t self)
{
  uint8_t i;
  if(!self) return;

  // free any locals created
  for(i=0; i<CS_MAX; i++)
  {
    if(!self->locals[i] || !e3x_cipher_sets[i]) continue;
    e3x_cipher_sets[i]->local_free(self->locals[i]);
    lob_free(self->keys[i]);
  }

  free(self);
  return;
}

// try to decrypt any message sent to us, returns the inner
lob_t e3x_self_decrypt(e3x_self_t self, lob_t message)
{
  e3x_cipher_t cs;
  if(!self || !message) return LOG("bad args");
  if(message->head_len != 1) return LOG("invalid message");
  cs = e3x_cipher_set(message->head[0],NULL);
  if(!cs) return LOG("no cipherset %2x",message->head[0]);
  return cs->local_decrypt(self->locals[cs->id],message);
}

// generate a signature for the data
lob_t e3x_self_sign(e3x_self_t self, lob_t args, uint8_t *data, size_t len)
{
  local_t local = NULL;
  e3x_cipher_t cs = NULL;
  char *alg = lob_get(args,"alg");
  if(!data || !len || !alg) return LOG("bad args");
  if(util_cmp(alg,"HS256") == 0) cs = e3x_cipher_set(0x1a,NULL);
  if(util_cmp(alg,"ES160") == 0) cs = e3x_cipher_set(0x1a,NULL);
  if(util_cmp(alg,"RS256") == 0) cs = e3x_cipher_set(0x2a,NULL);
  if(util_cmp(alg,"ES256") == 0) cs = e3x_cipher_set(0x2a,NULL);
  if(util_cmp(alg,"ED25519") == 0) cs = e3x_cipher_set(0x3a,NULL);
  if(!cs || !cs->local_sign) return LOG("no signing support for %s",alg);
  if(self) local = self->locals[cs->id];
  return cs->local_sign(local,args,data,len);
}
