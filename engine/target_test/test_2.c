
int global_array[128];
#if 0
int data1 __attribute__((section("data1_"))) = 0;
int data2 __attribute__((section("data2_"))) = 0;
int data3 __attribute__((section("data3_"))) = 0;
int data4 __attribute__((section("data4_"))) = 0;

void foo1(void) __attribute__((section("foo1_")));
void foo2(void) __attribute__((section("foo2_")));
void foo3(void) __attribute__((section("foo3_")));

void foo1(void){}
void foo2(void){}
void foo3(void){}
#endif
