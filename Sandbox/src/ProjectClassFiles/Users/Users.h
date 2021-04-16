#pragma once
#include <string>
#include <vector>
#include "ProjectClassFiles/Items/Item.h"

//class to store users, with information like if they are the supplier, and their purchase information
class Users
{
public:

public:
	std::string username;
	std::string password;
	std::vector<Item> cart;
	bool supplier = false;
	std::string address;
	std::string phone;
	int CreditCardNumber = -1;
	bool PremiumAccount = false;
	bool FirstAnnualPurchase = false;
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
		int supplier = 0;
		char address[8] = "tmp  st";
		char phone[11] = "0123456789";
		int CreditCardNumber = -1;
		int PremiumAccount = -1;
		int FirstAnnualPurchase = -1;

		int debuging = 0;
		if (0 == fscanf(in, "{\nusername: %s\n", username))
			debuging++;
		if (0 == fscanf(in, "password: %s\n", password))
			debuging++;
		if (0 == fscanf(in, "IsAdmin: %d\n", &supplier))
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


		allUsers.push_back({ username, password, {}, (bool)supplier, std::string(address) + " st", phone, CreditCardNumber, (bool)PremiumAccount, (bool)FirstAnnualPurchase });			//store the balance inside the map getting map[card number] = balance
	}

	fclose(in);
}

//sets users after program ends
static void setUsers()
{
	FILE* out = fopen("src\\ProjectClassFiles\\Users\\users.txt", "w");

	Users customer;
	for (int i = 0; i < allUsers.size(); i++)
	{
		customer = allUsers[i];
		int debuging = 0;
		if (0 == fprintf(out, "{\nusername: %s\n", customer.username.c_str()))
			debuging++;
		if (0 == fprintf(out, "password: %s\n", customer.password.c_str()))
			debuging++;
		if (0 == fprintf(out, "IsAdmin: %d\n", customer.supplier))
			debuging++;
		if (0 == fprintf(out, "address: %s\n", customer.address.c_str()))
			debuging++;
		if (0 == fprintf(out, "phone :%s\n", customer.phone.c_str()))
			debuging++;
		if (0 == fprintf(out, "Credit card number: %d\n", customer.CreditCardNumber))
			debuging++;
		if (0 == fprintf(out, "Premium Account: %d\n", customer.PremiumAccount))
			debuging++;
		if (0 == fprintf(out, "First Annual Purchase: %d\n}", customer.FirstAnnualPurchase))
			debuging++;

		if (i != allUsers.size() - 1)
			fprintf(out, "\n");
	}

	fclose(out);
}

//returns if the username exist in current users
static bool findUsername(std::string username)
{
	for (auto& user : allUsers)
	{
		if (0 == strcmp(user.username.c_str(), username.c_str()))
			return true;
	}
	return false;
}