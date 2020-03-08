//
//  main.cpp
//  Assignment No. 2
//
//  Created by Jonathan Joshua on 9/24/17.
//  Copyright © 2017 Jonathan Joshua. All rights reserved.
//

#include <iostream>
using namespace std;

int main()
{
    //create variables first, and then use them
    float num1, answer;
    
    //let's go grab a number from the users
    cout << "Give me a number: " << endl;
    cin >> num1;
    
    //let's do some math with this number
    answer = num1 * 2;
    
    //print back some magic
    cout << "Answer: " << answer << endl;
    
    return 1;
}

