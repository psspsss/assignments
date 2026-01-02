StdAttendance *sArr[] = {new ArtsStudent("Smitha", 68),
                         new ScienceStudent("Rahul", 72),
                         new EngineeringStudent("Sachin", 78),
                         new ArtsStudent("Balu", 64),
                         new ScienceStudent("Sharma", 72),
                         new EngineeringStudent("Rathore", 79),
                         NULL};

for (int cnt = 0; sArr[cnt] != NULL; cnt++) {
  cout << sArr[cnt]->getName() << " is ";
  if (sArr[cnt]->checkEligibility())
    cout << "Eligilible" << endl;
  else
    cout << "Not Eligilible" << endl;
}
