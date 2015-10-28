#include <iostream>
#include <vector>
#include <string>
#include <fstream>

using namespace std;

int addNumbers(int firstNum, int secondNum = 0){
	int combinedValue = firstNum + secondNum;
	return combinedValue;
}

int addNumbers(int firstNum, int secondNum, int thirdNum){
	int combinedValue = firstNum + secondNum + thirdNum;
	return combinedValue;
}

int getFactorial(int number){
	int sum;
	if(number == 1) sum = 1;
	else sum = getFactorial(number - 1) * number;
	return sum;
}

void makeMeYoung(int* age){
	*age = 21;
}

void actYourAge(int &age){
	age = 23;
}

int main(){
	{
		cout << addNumbers(1) << endl;
		cout << addNumbers(1, 5, 6) << endl; 
		cout << "the factorial of 3 is " << getFactorial(3) << endl;
	}

	{
		string steveQuote = "a day without sunshine is like, you know, night";
		ofstream writer("stevequote.txt");
		if (! writer){
			cout << "error opening file" << endl;
			return -1;
		}else{
			writer << steveQuote << endl;
			writer.close();
		}

		char letter;

		ifstream reader("stevequote.txt");
		if (! reader){
			cout << "error opening file" << endl;
			return -1;
		}else{
			for (int i = 0; !reader.eof(); i++){
				reader.get(letter);
				cout << letter;
			}
			cout << endl;
			reader.close();
		}

	}


	{
		int number = 0;
		try {
			if (number != 0){
				cout << 2/number << endl;
			}else 
				throw(number);
		}

		catch(int number){
			cout << number << " is not valid " << endl;
		}
	}


	{
		int myAge = 39;
		char myGrade = 'A';
		cout << "size of int " << sizeof(myAge) << endl;
		cout << "size of char " << sizeof(myGrade) << endl;

		cout << "myAge is locate at " << &myAge << endl;
	}

	{
		int myAge = 39;
		int* agePtr = &myAge;
		cout << "address of pointer " <<  agePtr << endl;
		cout << "data at memory address " << *agePtr << endl;

		int badNums[5] = {4, 13, 14, 24, 34};
		int* numArrayPtr = badNums;

		cout << "address " << numArrayPtr << " Value " << *numArrayPtr << endl;

		numArrayPtr++;

		cout << "address " << numArrayPtr << " Value " << *numArrayPtr << endl;
		cout << "address " << badNums << " Value " << *badNums << endl;
		makeMeYoung(&myAge);
		cout << "i'm " << myAge << " years old now" << endl;

		int &ageRef = myAge;
		cout << "&ageRef " << &ageRef << " &myAge " << &myAge << endl;
		cout << "myAge: " << myAge << endl;
		ageRef++;
		cout << "myAge: " << myAge << endl;

		actYourAge(ageRef);
		cout << "myAge: " << myAge << endl;
	}


	return 0;
}
