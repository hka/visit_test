#include "helper.h"

#include <visit_struct/visit_struct.hpp>
#include <vector>


struct A
{
  int a;
  double b;
};
VISITABLE_STRUCT(A, a, b);

struct B
{
  int a;
  std::vector<int> b;
  A c;
};
VISITABLE_STRUCT(B, a, b, c);

int main(int argc, char* argv[])
{
  B b = {
    .a = 2,
    .b = {1, 2, 3},
    .c = {
      .a = 4,
      .b = 2.1,
    }
  };
  serialize(b, "test.json");
  B c;
  deserialize(c, "test.json");
  serialize(c, "test2.json");
  if(struct_eq(b, c))
    printf("True\n");
  else
    printf("False\n");
  return 0;
}
