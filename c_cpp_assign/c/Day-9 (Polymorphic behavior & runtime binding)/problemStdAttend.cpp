/*
 * Purpose: An attendance system manages students of different programs: Engineering, 
 * Arts, and Science. Each student has a name and attendance percentage.
 *
 *
 */
#include <iostream>
using namespace std;

class StdAttendance{
protected:
	string name;
	double attendance;
public:
	StdAttendance(const string &n, double attn):name(n), attendance(attn){} 	
	virtual bool checkEligibility() = 0;
	string getName(){return name;}
};

class EngineeringStudent: public StdAttendance{
public:
	EngineeringStudent(const string &n, double att): StdAttendance(n, att) {} 
	bool checkEligibility(){
		return attendance >= 75;
	}
};
class ArtsStudent: public StdAttendance{
public:
	ArtsStudent(const string &n, double att): StdAttendance(n, att) {} 
	bool checkEligibility(){
		return attendance >= 65;
	}
};
class ScienceStudent: public StdAttendance{
public:
	ScienceStudent(const string &n, double att): StdAttendance(n, att) {} 
	bool checkEligibility(){
		return attendance >= 70;
	}
};


int main(){
	StdAttendance *sArr[] = {new ArtsStudent("Smitha", 68), new ScienceStudent("Rahul", 72), new EngineeringStudent("Sachin", 78), new ArtsStudent("Balu", 64), new ScienceStudent("Sharma", 72),new EngineeringStudent("Rathore", 79), nullptr};

	
	for(int cnt =0 ;sArr[cnt] != nullptr; cnt++){
		cout<<sArr[cnt]-> getName()<<" is ";
		if(	sArr[cnt]->checkEligibility())
			cout<<"Eligible"<<endl;
		else
			cout<<"Not Eligible"<<endl;
	}
	

	/*
	
	StdAttendance *s = new EngineeringStudent("Rahul", 78);
	cout<<s->getName()<<" is ";
	if(	s->checkEligibility())
		cout<<"Eligible"<<endl;
	else
		cout<<"Not Eligible"<<endl;
	*/
}
