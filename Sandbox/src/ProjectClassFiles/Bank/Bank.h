#pragma once
#include "fstream"
#include <sstream>
#include <map>

class Bank
{
public:
	Bank()
	{
		//stores all the accounts in the account file inside in
		FILE *in = fopen("src\\ProjectClassFiles\\Bank\\Accounts.txt", "r");

		//while we haven't reached the end of the file
		while (!feof(in))
		{		//store account balance

			int cardnum, bal;
			if (!fscanf(in, "%d->%d", &cardnum, &bal));


			accounts[cardnum] = bal;			//store the balance inside the map getting map[card number] = balance

		}
		fclose(in);
	}

	//charges the card the price
	void ChargeAmount(int CreditCard, int price)
	{		
		bool found = false;//set found to false since we haven't found the card number in our system
		int* balance;//store the memory address of the balance of that card number
		
		for (auto& account : accounts)//loop through all accounts
		{
			if (account.first == CreditCard)//if the account card number matches the one we are looking for
			{
				found = true;//set found to true and store the balance's memory address, and break loop since we got what we wanted
				balance = &account.second;
				break;
			}
		}

		//tracks total number of orders taken place, used as conformation number if purchase is sucessfull
		cf++;

		//if we found the card, try and charge them, else set the conformation number to -1
		if (found)
		{
			//only charge them if they have enough on their card
			bool charged = (*balance >= price);
			//since booleans equate to 1 or zero if they are true or false, multiplying by the bool will only change value if true
			*balance -= charged * price;
			//increases
			ConformationNumber = (charged)?cf:-1;
		}
		else
		{
			ConformationNumber = -1;
		}


		//set request to false since we processed this
		Request = false;
	}
	unsigned long ConformationNumber = 1;
	bool Request = false;
	std::map<int, int> accounts;
	unsigned long cf = 0;
};

