#pragma once
#include <string>


class Item
{
public:
	std::string name;
	int amount;
	std::string discription;
	int regPrice, premPrice;

	Item(std::string _name, int _quantity, std::string _discription, int _regPrice, int _premprice)
		:name(_name), amount(_quantity), discription(_discription), regPrice(_regPrice), premPrice(_premprice)
	{}

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

	std::string getRegPrice()
	{
		return GetPrice(regPrice * amount);
	}

	std::string getPremPrice()
	{
		return GetPrice(premPrice * amount);
	}

	std::string price(bool premium)
	{
		if (premium)
			return getPremPrice();
		return getRegPrice();
	}
};


//static variable to store the stores inventory.
static std::vector<Item> inventory;
