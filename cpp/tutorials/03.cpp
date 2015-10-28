#include <iostream>
#include <vector>
#include <string>
#include <fstream>

using namespace std;


class Animal{
	private:
		int height;
		int weight;
		string name;

		static int numOfAnimals;

	public:
		int getHeight(){ return height;}
		int getWeight(){ return weight;}
		string getName(){ return name; }
		void setHeight(int cm){ height = cm; }
		void setWeight(int kg){ weight = kg; }
		void setName(string animalName){ name = animalName; }

		void setAll(int, int, string);

		Animal(int, int, string);
		Animal();

		~Animal();


		static int getNumOfAnimals() { return numOfAnimals; }

		void toString();
};


int Animal::numOfAnimals = 0;

void Animal::setAll(int height, int weight, string name){
	this->height = height;
	this->weight = weight;
	this->name = name;
	Animal::numOfAnimals++;
}

Animal::Animal(int height, int weight, string name){
	this->height = height;
	this->weight = weight;
	this->name = name;
	Animal::numOfAnimals++;
}

Animal::Animal(){
	Animal::numOfAnimals++;
}

Animal::~Animal(){
	cout << "Animal " << this->name << " destroyed " << endl;
}


void Animal::toString(){
	cout << this -> name << " is " << this->height << " cms tall and " 
		<< this-> weight << " kgs in weight " << endl;
}

class Dog:public Animal {
	private:
		string sound = "Woof";
	
	public:
		void getSound() { cout << sound << endl;}

		Dog(int, int, string, string);
		Dog():Animal(){};
		void toString();
};

Dog::Dog(int height, int weight, string name, string bark):Animal(height, weight, name){
	this -> sound = bark;
}



void Dog::toString(){
	cout << this->getName() << " is " << this->getHeight() <<
		" cms tall and " << this->getWeight() << 
		" kgs in weight and says " << this->sound << endl;
}

int main(){
	Animal fred; 
	fred.setHeight(33);
	fred.setWeight(10);
	fred.setName("Fred");
	cout << fred.getName() << " is " << fred.getHeight() <<
		" cms tall and " << fred.getWeight() << " kgs n weight" << endl;

	Animal tom(36, 15, "Tom");
	cout << tom.getName() << " is " << tom.getHeight() <<
		" cms tall and " << tom.getWeight() << " kgs n weight" << endl;

	Dog spot(38, 16, "Spot", "Wooooof");
	cout << "Number of Animals " << Animal::getNumOfAnimals() << endl;

	spot.getSound();
	tom.toString();
	spot.toString();
	spot.Animal::toString();
	return 0;
}
