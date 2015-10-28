#include <iostream>
#include <vector>
#include <string>
#include <fstream>

using namespace std;

int main(){
	{
		cout << "hello world" << endl;
	}

	{
		const double PI = 3.141526535;
		char myGrade = 'A';
		bool isHappy = true;
		int myAge = 39;
		float favNum = 3.141592;
		double otherfavNum = 1.6180339887;
		cout << "Favorite Number" << favNum << endl;
		cout << "Size of int " << sizeof(myAge) << endl;
		cout << "Size of char " << sizeof(myGrade) << endl;
		cout << "Size of bool " << sizeof(isHappy) << endl;
		cout << "Size of float " << sizeof(favNum) << endl;
		cout << "Size of double " << sizeof(otherfavNum) << endl;
		cout << "Size of int " << sizeof(myAge) << endl;
	}

	{
		int five = 5;
		cout << "5++ " << five++ << endl;
		cout << "++5 " << ++five << endl;
		cout << "5-- " << five-- << endl;
		cout << "--5 " << --five << endl;
	}

	{
		cout << "1 + 2 - 3 * 2 = " << 1 + 2 - 3 * 2 << endl;
		cout << "(1 + 2 - 3) * 2 = " << (1 + 2 - 3) * 2 << endl;
		cout << "4 / 5 = " << 4 / 5 << endl;
		cout << "4 / 5 = " << (float)4 / 5 << endl;
	}

	{
		int age = 70;
		int ageAtLastExam = 16;
		bool isNotIntoxicated = true;
		if((age >= 1) && (age < 16)){
			cout << "You can't drive" << endl;
		}else if (! isNotIntoxicated){
			cout << "You can't drive" << endl;
		}else if ((age >= 80) && ((age > 100)|| ((age - ageAtLastExam) > 5))){
			cout << "You can't drive" << endl;
		}else{
			cout << "You can drive" << endl;
		}
	}


	{
		int greetingOption = 2;
		switch(greetingOption){
			case 1:
				cout << "bonjour" << endl;
				break;
			case 2:
				cout << "hola" << endl;
				break;
			case 3:
				cout << "hallo" << endl;
				break;
			default:
				cout << "hello" << endl;
				break;
		}
	}


	/* */
	{
		int myFavNums[5];
		int badNums[5] = {4, 13, 14, 24, 34};
		cout << "Bad Number 1: " << badNums[0] << endl;
		char myName[5][5] = {{'D', 'e', 'r', 'e', 'k'},
			{'B', 'a', 'n', 'a', 's'}};
		cout << "2nd letter in 2nd array " << myName[1][1] << endl;
		myName[0][2] = 'e';
		cout << "new value " << myName[0][2] << endl;
		for(int i = 1; i <= 10; i++){
			cout << i << endl;
		}

		for (int j = 0; j < 5; j++){
			for (int k = 0; k < 5; k++){
				cout << myName[j][k];
			}
		}
		cout << endl;
	}


	{
		int randNum = (rand() % 100) + 1;
		while (randNum != 100){
			cout << randNum << ",";
			randNum = (rand() % 100) + 1;
		}
		cout << endl;
	}

	{
		int index = 1;
		while(index <= 10){
			cout << index << endl;
			index++;
		}
	}

	/*
	{
		string numberGuessed;
		int intNumberGuessed = 0;
		do{
			cout << "Guess between 1 and 10:";
			getline(cin, numberGuessed);
			intNumberGuessed = stoi(numberGuessed);
			cout << intNumberGuessed << endl;
		}while(intNumberGuessed != 4);
		cout << "you win" << endl;
	}
	*/

	{
		 char happyArray[6] = {'H', 'a', 'p', 'p', 'y', '\0'};
		 string birthdayString = " birthday";
		 cout << happyArray + birthdayString << endl;
		 string yourName;
		 //cout << "what is your name ";
		 //getline(cin, yourName);
		 //cout << "hello " << yourName << endl;
		 yourName = "derek Banas";

		 /*
		 double euleronstant = .57721;
		 string eulerGuess;
		 double eulerGuessDouble;

		 cout << "what is Euler's constant? ";
		 getline(cin, eulerGuess);
		 eulerGuessDouble = stod(eulerGuess);
		 if(eulerGuessDouble == eulerConstant){
			 cout << "you are right" << endl;
		 }else{
			 cout << "you are wrong" << endl;
		 }

		 cout << "size of string " << eulerGuess.size() << endl;
		 cout << "is string empty " << eulerGuess.empty() << endl;
		 cout << eulerGuess.append(" war your guess") << endl;
		 */

		 {
		 string dogString = "dog";
		 string catString = "cat";
		 cout << dogString.compare(catString) << endl;
		 cout << dogString.compare(dogString) << endl;
		 cout << catString.compare(dogString) << endl;
		 }

		 {
			 string wholeName = yourName.assign(yourName);
			 cout << wholeName << endl;

			 string firstName = wholeName.assign(wholeName, 0, 5);
			 cout << firstName << endl;

			 int lastNameIndex = yourName.find("Banas", 0);
			 cout << "Index for last name " << lastNameIndex << endl;

			 yourName.insert(5, " Justin");
			 cout << yourName << endl;

			 yourName.erase(6,7);
			 cout << yourName << endl;

			 yourName.replace(6, 5, "maximus");
			 cout << yourName << endl;
		 }

	}


	{
		vector <int> lotteryNumVect(10);
		int lotteryNumArray[5] = {4, 13, 14, 24, 34};
		cout << "size " << lotteryNumVect.size() << endl;
		lotteryNumVect.insert(lotteryNumVect.begin(), lotteryNumArray, 
				lotteryNumArray+3);
		cout << lotteryNumVect.at(2) << endl;
		cout << "size " << lotteryNumVect.size() << endl;

		lotteryNumVect.insert(lotteryNumVect.begin() + 5, 44);
		cout << "size " << lotteryNumVect.size() << endl;
		lotteryNumVect.push_back(64);
		cout << "final value " << lotteryNumVect.back() << endl;
		cout << "fron " << lotteryNumVect.front() << endl;
		cout << "size " << lotteryNumVect.size() << endl;
		cout << "all " << lotteryNumVect.at(14) << endl;

	}

	return 0;
}
