#pragma once
#include "ProjectClassFiles/Items/Item.h"

enum status { Processing = 0, Shipped, Pickup };

class invoice
{
public:
	std::string user;
	bool premium;
	std::vector<Item> cart;
	std::string date;
	int total, card;
	status currentStatus;
	unsigned long conformationCode = -1;
};

std::vector<invoice> allOrders;
static unsigned long GetAllOrders()
{
	//stores all the orders in the order file inside in
	FILE* in = fopen("src\\ProjectClassFiles\\Invoice\\allOrders.txt", "r");

	//stores the latest(largest) conformation code, to assign back to bank
	unsigned long conformationCode = 0;

	//while we haven't reached the end of the file
	while (!feof(in))
	{		//store account balance
		char user[5] = { 0 };
		std::vector<Item> cart;
		char date[11] = { 0 };
		int total, card, numberOfItems;
		int debuging = 0;
		int premium;
		int current;

		if (0 == fscanf(in, "{\n\tuser: %4s\n", user))
			debuging++;
		if (0 == fscanf(in, "\tprem: %d\n", &premium))
			debuging++;
		if (0 == fscanf(in, "\tnumber of items %d\n", &numberOfItems))
			debuging++;

		for (int i = 0; i < numberOfItems; i++)
		{
			int amount, regPrice, premPrice;
			char name[51] = { 0 }, description[501] = { 0 };


			if (0 == fscanf(in, "\t[\nname: %50[^\n]\n", name))
				debuging++;
			if (0 == fscanf(in, "\tquantity: %d\n", &amount))
				debuging++;
			if (0 == fscanf(in, "\tdescription: %500[^\n]\n", description))
				debuging++;
			if (0 == fscanf(in, "\tregular price: %d\n", &regPrice))
				debuging++;
			if (0 == fscanf(in, "\tpremium price: %d\n]\n", &premPrice))
				debuging++;

			cart.push_back({ name, amount, description, regPrice, premPrice });
		}


		if (0 == fscanf(in, "\tdate: %10s\n", date))
			debuging++;
		if (0 == fscanf(in, "\ttotal: %d\n", &total))
			debuging++;
		if (0 == fscanf(in, "\tcard#: %d\n", &card))
			debuging++;
		if (0 == fscanf(in, "\tstatus: %d\n", &current))
			debuging++;
		if (0 == fscanf(in, "\tConformationCode: %lu\n}\n", &conformationCode))
			debuging++;
		if (debuging == 0)
			allOrders.push_back({ std::string(user), (bool)premium, cart, std::string(date), total, card, (status)current, conformationCode });			//store the balance inside the map getting map[card number] = balance
		else
			1 / debuging;
	}

	fclose(in);

	return conformationCode;
}


static void SetAllOrders()
{
	//stores all the orders in the order file inside in
	FILE* in = fopen("src\\ProjectClassFiles\\Invoice\\allOrders.txt", "w");

	//while we haven't reached the end of the file
	for (auto& order : allOrders)
	{
		int debuging = 1;

		if (0 == fprintf(in, "{\n\tuser: %s\n", order.user.c_str()))
			debuging++;
		if (0 == fprintf(in, "\tprem: %d\n", (int)order.premium))
			debuging++;
		if (0 == fprintf(in, "\tnumber of items %d\n", (int)order.cart.size()))
			debuging++;

		for (int i = 0; i < order.cart.size(); i++)
		{

			if (0 == fprintf(in, "\t[\n\t\tname: %s\n", order.cart[i].name.c_str()))
				debuging++;
			if (0 == fprintf(in, "\t\tquantity: %d\n", order.cart[i].amount))
				debuging++;
			if (0 == fprintf(in, "\t\tdescription: %s\n", order.cart[i].description.c_str()))
				debuging++;
			if (0 == fprintf(in, "\t\tregular price: %d\n", order.cart[i].regPrice))
				debuging++;
			if (0 == fprintf(in, "\t\tpremium price: %d\n\t]\n", order.cart[i].premPrice))
				debuging++;
		}


		if (0 == fprintf(in, "\tdate: %10s\n", order.date.c_str()))
			debuging++;
		if (0 == fprintf(in, "\ttotal: %d\n", order.total))
			debuging++;
		if (0 == fprintf(in, "\tcard#: %d\n", order.card))
			debuging++;
		if (0 == fprintf(in, "\tstatus: %d\n", (int)order.currentStatus))
			debuging++;
		if (0 == fprintf(in, "\tConformationCode: %lu\n}\n", order.conformationCode))
			debuging++;

	}

	fclose(in);
}