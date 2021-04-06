#pragma once
#include <string>
#include <vector>

class Users
{
public:

public:
	std::string username;
	std::string password;
	bool IsAdmin;
	std::string address;
	std::string phone;
	int CreditCardNumber;
	bool PremiumAccount;
	bool FirstAnnualPurchase;
};


std::vector<Users> allUsers;
static void getUsers()
{
	//stores all the accounts in the account file inside in
	FILE* in = fopen("src\\ProjectClassFiles\\Users\\users.txt", "r");

	//while we haven't reached the end of the file
	while (!feof(in))
	{		//store account balance
		char username[5] = "tmp ";
		char password[5] = "tmp ";
		int IsAdmin = 0;
		char address[8] = "tmp  st";
		char phone[11] = "0123456789";
		int CreditCardNumber = -1;
		int PremiumAccount = -1;
		int FirstAnnualPurchase = -1;

		std::string fstring = "{\n"
			"username: %s\n"
			"password: %s\n"
			"IsAdmin: %d\n"
			"address: %s st\n"
			"phone :%s\n"
			"Credit card number: %d\n"
			"Premium Account: %d\n"
			"First Annual Purchase: %d\n"
			"}";

		int debuging = 0;
		if (0 == fscanf(in, "{\nusername: %s\n", username))
			debuging++;
		if (0 == fscanf(in, "password: %s\n", password))
			debuging++;
		if (0 == fscanf(in, "IsAdmin: %d\n", &IsAdmin))
			debuging++;
		if (0 == fscanf(in, "address: %s st\n", address))
			debuging++;
		if (0 == fscanf(in, "phone :%s\n", phone))
			debuging++;
		if (0 == fscanf(in, "Credit card number: %d\n", &CreditCardNumber))
			debuging++;
		if (0 == fscanf(in, "Premium Account: %d\n", &PremiumAccount))
			debuging++;
		if (0 == fscanf(in, "First Annual Purchase: %d\n}\n", &FirstAnnualPurchase))
			debuging++;


		allUsers.push_back({ username, password, (bool)IsAdmin, std::string(address) + " st", phone, CreditCardNumber, (bool)PremiumAccount, (bool)FirstAnnualPurchase });			//store the balance inside the map getting map[card number] = balance

	}
}
