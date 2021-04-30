#pragma once
#include <string>
#include <vector>

//helper functions to convert price string (ex. $20.26) into int value in pennies (ex. 2026) to avoid floating point errors, and vice versa below
static std::string GetPrice(int p)
{
	return "$" + std::to_string(p / 100) + "." + std::to_string(p % 100);
}

static int GetPrice(std::string p)
{
	int total = 0;
	p = p.substr(1);
	total += std::stoi(p.substr(p.find(".") + 1));
	total += std::stoi(p) * 100;
	return total;;
}

//stores individual items
class Item
{
public:
	std::string name;
	int amount, reserved;
	std::string description;
	int regPrice, premPrice;

	Item(std::string _name, int _quantity, int _reserved, std::string _discription, int _regPrice, int _premprice)
		:name(_name), amount(_quantity), reserved(_reserved), description(_discription), regPrice(_regPrice), premPrice(_premprice)
	{}

	//return total price for given amount for regular accounts
	std::string getRegPrice()
	{
		return GetPrice(regPrice * amount);
	}

	//return total price for given amount for premium accounts
	std::string getPremPrice()
	{
		return GetPrice(premPrice * amount);
	}

	//returns price, calculating which we need on the boolean premium
	std::string price(bool premium)
	{
		if (premium)
			return getPremPrice();
		return getRegPrice();
	}
};


//static variable to store the stores inventory.
static std::vector<Item> inventory;


//gets all orders on startup
static void GetInventory()
{
	//stores all the items in the order file inside in
	FILE* in = fopen("Inventory.txt", "r");

	//while we haven't reached the end of the file
	while (!feof(in))
	{		
		//store item information
		char premium[501];
		//used to check if fscanf cant scan in a variable
		int debuging = 0;


		int amount, reserved, regPrice, premPrice;
		char name[51] = { 0 }, description[501] = { 0 };


		if (0 == fscanf(in, "{\nname: %50[^\n]\n", name))
			debuging++;
		if (0 == fscanf(in, "quantity: %d\n", &amount))
			debuging++;
		if (0 == fscanf(in, "reserved: %d\n", &reserved))
			debuging++;
		if (0 == fscanf(in, "description: %500[^\n]\n", description))
			debuging++;
		if (0 == fscanf(in, "regPrice: %d\n", &regPrice))
			debuging++;
		if (0 == fscanf(in, "premPrice: %d\n}\n", &premPrice))
			debuging++;

		//if debugging wasnt altered, store order in allorders
		if (debuging == 0)
			inventory.push_back({ name, amount, reserved, description, regPrice, premPrice });
		else
			debuging--;
	}

	//close file
	fclose(in);
}


//writes all orders to file at end of program
static void SetInventory()
{
	//stores all the orders in the order file inside in
	FILE* in = fopen("Inventory.txt", "w");

	//while we haven't reached the end of the file
			//store item information
	int debuging = 0;

	for (auto item : inventory)
	{

		if (0 == fprintf(in, "{\nname: %s\n", item.name.c_str()))
			debuging++;
		if (0 == fprintf(in, "quantity: %d\n", item.amount))
			debuging++;
		if (0 == fprintf(in, "reserved: %d\n", item.reserved))
			debuging++;
		if (0 == fprintf(in, "description: %s\n", item.description.c_str()))
			debuging++;
		if (0 == fprintf(in, "regPrice: %d\n", item.regPrice))
			debuging++;
		if (0 == fprintf(in, "premPrice: %d\n}\n", item.premPrice))
			debuging++;
	}

	fclose(in);
}