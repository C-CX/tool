#include"manager.h"



class TEST : public Module
{
public:
  void init(){};
};


class TEST_2 : public Module
{
public:
  void init(){};
};

REGISTER_MODULE(TEST);
REGISTER_MODULE(TEST);
REGISTER_MODULE(TEST_2);
REGISTER_MODULE(TEST_2);

int main(int argc, char const *argv[])
{
    printf("run\n");
    return 0;
}