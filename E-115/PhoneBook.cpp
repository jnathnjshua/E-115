#include<iostream>
#include<string>
#include<fstream>
using namespace std; 

void addNewContact() { 
    //implement
}

void searchContact() { 
    //implement
}

void displayContacts() { 
    //implement
}



int main() 
{
    int choice; 
    
    while(1) { 
        cout << "Press 1 to add contact" << endl;
        cout << "Press 2 to search for a contact" << endl;
        cout << "Press 3 to display contacts" << endl;
        cout << "Anything else to quit" << endl;
        cin >> choice;
        
        switch(choice) { 
            case 1 : addNewContact();
                     break; 
                     
            case 2 : searchContact();
                     break;
                     
            case 3 : displayContacts();
                     break;
                
            default: exit(1);
        }
    }
    
    return 1;
}