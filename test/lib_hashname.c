#include "hashname.h"
#include "unit_test.h"

int main(int argc, char **argv)
{
  hashname_t hn;
  hn = hashname_str((uint8_t*)"uvabrvfqacyvgcu8kbrrmk9apjbvgvn2wjechqr3vf9c1zm3hv7g");
  fail_unless(hn);

  return 0;
}

