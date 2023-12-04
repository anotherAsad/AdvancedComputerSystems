#include <iostream>
#include <vector>
#include <string>

#include<unistd.h>               // for linux 

#define NL '\n'

struct testClass {
  	static int count;
  	int idx = 0;
  	std::string name = "";
  	int *ptr = NULL;
  
	testClass() {
		ptr = new int;
		idx = ++count;
		std::cout << "Object " << idx << " created" << std::endl;
	}
	
	testClass(std::string name) {
		ptr = new int;
		idx = ++count;
		this->name = name;
		std::cout << "Object " << idx << " created with name " << this->name << std::endl;
	}
	
	
	testClass(const testClass &tc) {
		*this = tc;
		this->ptr = new int;
		std::cout << "copy made, name then is: " << this->name << std::endl;
	}
	
	~testClass() {
		delete ptr;
		std::cout << "Object " << idx << " *" << name << "* " << " destroyed" << std::endl;
		
		idx = -1;
	}
	
};

int testClass::count = 0;

testClass apparatus() {
	testClass tc0;
	
	std::cout << "Exiting from apparatus()" << std::endl;
	return tc0;
}


testClass fmain() {
	testClass stubObj("stub");

	std::vector<testClass> test_vec;
	test_vec.reserve(100);
	
	test_vec.emplace_back("Lilo");
	test_vec.emplace_back("eagLe");
	
//	std::cout << test_vec[0].idx << std::endl;
	std::cout << "Exiting from fmain()" << std::endl;
	
	if(stubObj.idx == 3) {
		testClass goliath("goliath");
		return goliath;
	}
	else if(stubObj.idx == 2){
		testClass david("david");
		return david;
	}
	
	return testClass("grando");//test_vec[1];
}
	

int main() {
	testClass stack_obj = fmain();
	
	std::cout << "My name is: " << stack_obj.name << std::endl;
	
	std::cout << "Exiting from main()" << std::endl;
}

