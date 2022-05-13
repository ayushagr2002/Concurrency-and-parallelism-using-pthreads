## README

For compiling the program, run:

`gcc q1.c -lpthread -o q1`

To run:

`./q1`

Enter the input as specified in the assignment pdf.
Sample Input:

```
10 3 4
SMAI 0.8 3 2 0 2
NLP 0.95 4 1 0
CV 0.90 2 2 1 2
DSA 0.75 5 3 0 1 2
0.8 0 3 1 1
0.6 3 1 2 3
0.85 2 1 0 1
0.5 1 2 3 2
0.75 0 2 1 3
0.95 1 0 2 2
0.4 3 0 2 3
0.1 0 3 1 2
0.85 1 0 3 1
0.3 0 1 2 1
PRECOG 3 1
CVIT 4 2
RRC 1 3
```



Sample Output:

```
Student 0 has filled in preferences for course registration
Student 2 has filled in preferences for course registration
Student 8 has filled in preferences for course registration
TA 0 from lab PRECOG has been allocated to course SMAI for his 1st TA ship
Course SMAI has been allocated 3 seats
Student 0 has been allocated a seat in course SMAI
TA 1 from lab PRECOG has been allocated to course NLP for his 1st TA ship
TA 0 from lab CVIT has been allocated to course CV for his 1st TA ship
Course CV has been allocated 2 seats
Student 2 has been allocated a seat in course CV
Student 9 has filled in preferences for course registration
TA 2 from lab PRECOG has been allocated to course DSA for his 1st TA ship
Course DSA has been allocated 2 seats
Course NLP has been allocated 2 seats
Student 8 has been allocated a seat in course NLP
Student 3 has filled in preferences for course registration
Student 5 has filled in preferences for course registration
Student 7 has filled in preferences for course registration
Tutorial has started for course SMAI with 1 seats filled out of 3 seats
Tutorial has started for course CV with 1 seats filled out of 2 seats
Tutorial has started for course DSA with 0 seats filled out of 2 seats
Tutorial has started for course NLP with 1 seats filled out of 2 seats
Student 1 has filled in preferences for course registration
Student 4 has filled in preferences for course registration
Student 6 has filled in preferences for course registration
TA 0 from lab CVIT has completed the tutorial for course CV
TA 1 from lab PRECOG has completed the tutorial for course NLP
Student 2 has selected course CV permanently
TA 0 from lab PRECOG has completed the tutorial for course SMAI
Student 8 has selected course NLP permanently
Student 0 has selected course SMAI permanently
TA 2 from lab PRECOG has completed the tutorial for course DSA
TA 0 from lab CVIT has been allocated to course CV for his 2nd TA ship
Course CV has been allocated 1 seats
Lab PRECOG no longer has students available for TA ship
Course NLP doesn't have any TAs eligible and is removed from course offerings
TA 0 from lab RRC has been allocated to course SMAI for his 1st TA ship
Course SMAI has been allocated 3 seats
Student 5 has changed current preference from course NLP(priority 1) to course SMAI(priority 2)
Student 3 has changed current preference from course NLP(priority 1) to course CV(priority 2)
TA 1 from lab CVIT has been allocated to course DSA for his 1st TA ship
Student 4 has been allocated a seat in course SMAI
Student 5 has been allocated a seat in course SMAI
Student 7 has been allocated a seat in course SMAI
Course DSA has been allocated 4 seats
Student 1 has been allocated a seat in course DSA
Student 6 has been allocated a seat in course DSA
Tutorial has started for course CV with 0 seats filled out of 1 seats
Tutorial has started for course SMAI with 3 seats filled out of 3 seats
Tutorial has started for course DSA with 2 seats filled out of 4 seats
TA 0 from lab CVIT has completed the tutorial for course CV
TA 1 from lab CVIT has completed the tutorial for course DSA
TA 0 from lab RRC has completed the tutorial for course SMAI
Student 1 has withdrawn from course DSA
Student 1 has changed current preference from course DSA(priority 1) to course NLP(priority 2)
Student 1 has changed current preference from course NLP(priority 2) to course CV(priority 3)
Student 6 has withdrawn from course DSA
Student 6 has changed current preference from course DSA(priority 1) to course SMAI(priority 2)
Student 7 has withdrawn from course SMAI
Student 7 has changed current preference from course SMAI(priority 1) to course DSA(priority 2)
Student 4 has selected course SMAI permanently
Student 5 has selected course SMAI permanently
TA 1 from lab CVIT has been allocated to course CV for his 2nd TA ship
Course CV has been allocated 2 seats
Student 1 has been allocated a seat in course CV
Student 3 has been allocated a seat in course CV
TA 0 from lab RRC has been allocated to course SMAI for his 2st TA ship
Course SMAI has been allocated 2 seats
Student 6 has been allocated a seat in course SMAI
Student 9 has been allocated a seat in course SMAI
TA 2 from lab CVIT has been allocated to course DSA for his 1st TA ship
Course DSA has been allocated 1 seats
Student 7 has been allocated a seat in course DSA
Tutorial has started for course CV with 2 seats filled out of 2 seats
Tutorial has started for course SMAI with 2 seats filled out of 2 seats
Tutorial has started for course DSA with 1 seats filled out of 1 seats
TA 0 from lab RRC has completed the tutorial for course SMAI
TA 1 from lab CVIT has completed the tutorial for course CV
Student 9 has withdrawn from course SMAI
TA 2 from lab CVIT has completed the tutorial for course DSA
Student 3 has selected course CV permanently
Student 1 has withdrawn from course CV
Student 1 couldn't get any of his preferred courses
Student 6 has withdrawn from course SMAI
Student 6 has changed current preference from course SMAI(priority 2) to course CV(priority 3)
Student 9 has changed current preference from course SMAI(priority 1) to course NLP(priority 2)
Student 7 has withdrawn from course DSA
Student 7 has changed current preference from course DSA(priority 2) to course NLP(priority 3)
Student 7 couldn't get any of his preferred courses
Student 9 has changed current preference from course NLP(priority 2) to course CV(priority 3)
TA 0 from lab RRC has been allocated to course SMAI for his 3rd TA ship
Course SMAI has been allocated 1 seats
TA 2 from lab CVIT has been allocated to course CV for his 2nd TA ship
Course CV has been allocated 1 seats
Student 6 has been allocated a seat in course CV
TA 3 from lab CVIT has been allocated to course DSA for his 1st TA ship
Course DSA has been allocated 5 seats
Tutorial has started for course SMAI with 0 seats filled out of 1 seats
Tutorial has started for course CV with 1 seats filled out of 1 seats
Tutorial has started for course DSA with 0 seats filled out of 5 seats
TA 0 from lab RRC has completed the tutorial for course SMAI
TA 2 from lab CVIT has completed the tutorial for course CV
TA 3 from lab CVIT has completed the tutorial for course DSA
Student 6 has selected course CV permanently
Course SMAI doesn't have any TAs eligible and is removed from course offerings
Lab CVIT no longer has students available for TA ship
Course CV doesn't have any TAs eligible and is removed from course offerings
Student 9 couldn't get any of his preferred courses
Course DSA doesn't have any TAs eligible and is removed from course offerings
Lab RRC no longer has students available for TA ship

Results of the Simulation:
Student 0 got course SMAI(Priority: 1)
Student 1 did not get any of his preferred courses
Student 2 got course CV(Priority: 1)
Student 3 got course CV(Priority: 2)
Student 4 got course SMAI(Priority: 1)
Student 5 got course SMAI(Priority: 2)
Student 6 got course CV(Priority: 3)
Student 7 did not get any of his preferred courses
Student 8 got course NLP(Priority: 1)
Student 9 did not get any of his preferred courses
```

