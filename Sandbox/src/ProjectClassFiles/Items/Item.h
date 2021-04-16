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
	int amount;
	std::string description;
	int regPrice, premPrice;

	Item(std::string _name, int _quantity, std::string _discription, int _regPrice, int _premprice)
		:name(_name), amount(_quantity), description(_discription), regPrice(_regPrice), premPrice(_premprice)
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
