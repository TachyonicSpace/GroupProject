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
		std::string str;
		std::ifstream in("src\\ProjectClassFiles\\Bank\\Accounts.txt");

		//while we haven't reached the end of the file
		while (in.peek() != std::ifstream::traits_type::eof())
		{
			//store the next line in str
			in >> str;
			//extract the card number from the string
			int cardnum = std::stoi(str.substr(0, str.find("->")));
			//remove the card number, and the number separator from string
			str = str.substr(str.find("->") + 2);
			//store account balance
			int bal = std::stoi(str);
			//store the balance inside the map getting map[card number] = balance
			accounts[cardnum] = bal;
		}
	}

	//charges the card the price
	void ChargeAmount(int CreditCard, int price)
	{
		//todo: remove
		if (Request)
			std::cout << "charging card\t";

		
		//set found to false since we haven't found the card number in our system
		bool found = false;
		//store the memory address of the balance of that card number
		int* balance;
		//loop through all accounts
		for (auto& account : accounts)
		{
			//if the account card number matches the one we are looking for
			if (account.first == CreditCard)
			{
				//set found to true and store the balance's memory address, and break loop
				found = true;
				balance = &account.second;
				break;
			}
		}
		//if we found the card, try and charge them, else set the conformation number to -1
		if (found)
		{
			//only charge them if they have enough on their card
			bool charged = (*balance > price);
			//since booleans equate to 1 or zero if they are true or false, multiplying by the bool will only change value if true
			*balance -= charged * price;
			//store a random number
			int tmp = rand();
			//set the conformation number to a random value that isnt -1;
			ConformationNumber = (charged)?((tmp!=-1)?tmp:tmp*-1 ): -1;
		}
		else
			ConformationNumber = -1;


		//set request to false since we processed this
		Request = false;

	}
	unsigned long ConformationNumber = 0x123456789abcdef;
	bool Request = false;
	std::map<int, int> accounts;
private:
};

