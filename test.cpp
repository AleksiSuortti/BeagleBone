#include <iostream>
#include <chrono>
#include <thread>


int main() {
	
	//start measuring time
	auto start = std::chrono::high_resolution_clock::now();
	
    for(int i = 0; i < 10000; i++) {
        std::cout << i << std::endl;
    }
    
    //time at the end of the program
    auto end = std::chrono::high_resolution_clock::now();
    
    //Duration to run the program
    std::chrono::duration<double> duration = end - start;
    
    std::cout << "Time taken to run: " << duration.count() << " seconds." << std::endl;
    
    return 0;
}
