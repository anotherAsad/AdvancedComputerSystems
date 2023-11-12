#include <iostream>
#include <string>
#include <thread>

typedef void funcptr_t(int &);

class stubClass {
  public:
	stubClass *stubClassArr[5];
	int abc[5];

	stubClass() {
		std::cout << "Hi" << std::endl;
	}

	static void getInt(int &a) {
		a++;
	}

	static std::string my_string;
};

funcptr_t *funcptr;

std::string stubClass::my_string;

void threadFunc(int *a) {
	std::cout << "Hello from " << *a << std::endl;
}

int main() {
	stubClass stubClassInst;
	stubClass stubClassArr[5];
	stubClassInst.stubClassArr[0] = new stubClass();

	funcptr = stubClass::getInt;

	int a = 6;

	funcptr(a);

	std::cout << a << std::endl;

	
	stubClass::my_string.push_back('a');

	std::thread t1(threadFunc, &a);
	std::thread t2(threadFunc, &a);
	
	t1.join();
	t2.join();


	std::cout << stubClass::my_string << std::endl;
}