#include "ImGui/ImGuiLayer.h"

#include "imgui.h"
#include "iostream"
#include "ProjectClassFiles/Bank/Bank.h"
#include <thread>

//temporary
class Items
{
public:
	Items(int Amount, std::string Name, int Price)
		:amount(Amount), name(Name), priceInPennies(Price) {}
	int amount;
	std::string name;
	std::string price()
	{
		int p = amount * priceInPennies;
		return "$" + std::to_string(p / 100) + "." + std::to_string(p % 100);
	}
private:
	int priceInPennies;
};
class user
{
public:
	bool admin = false;
	int CC = 123;
	bool Premium = false;
	bool firstAnnualPurchase = false;
};
user tmpAccount;



//TODO: implement in customer's account
std::vector<Items> cart;
//TODO: replace with a user ptr
bool loggedin = false;

//store the account information the banking system needs
int creditCard = 121, chargingAmount = 1000;//used by banking system
Bank* bank = new Bank();
Application* app = new Application();//needed to see when program ends


//this thread will run seperate of the main thread until the lambda funtion stops, ie the program stops
std::thread bankingSystem([&] {
	while (app->m_Running)
	{
		//if the bank has a request, charge the amount to the card, else wait
		if (bank->Request)
			bank->ChargeAmount(creditCard, chargingAmount);
	}
	});




//renders the item
void OnImGuiRender(Application& app)
{
	//docking in imguiLayer::begin()

	//menu
	if (ImGui::BeginMenuBar())
	{
		//adds a menu to the top for account settings
		if (ImGui::BeginMenu("Account Settings"))
		{
			//ends program
			if (ImGui::MenuItem("Quit"))
				app.m_Running = false;

			if (loggedin)
			{
				if (ImGui::MenuItem("logout"))
					loggedin = false;

				if (tmpAccount.admin)
					ImGui::MenuItem("Browse Catalog");
			}
			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}

	//begins a page for the store UI, like logging in and looking at items offered
	ImGui::Begin("Store");
	{
		//string we need to store the input text from imgui
		static std::string str = "";
		//bool to see if we add on a charge for the premium account
		bool addPremiumPrice = tmpAccount.Premium && tmpAccount.firstAnnualPurchase;

		//if the customer is logged in, then they will see these options
		if (loggedin)
		{
			//boolean to see if we are adding an item to the cart
			bool enter;
			//TODO: replace with a filter to filter items in a list of catalog items
			enter = ImGui::InputText("hello", &str[0], 255, ImGuiInputTextFlags_EnterReturnsTrue);			ImGui::SameLine();

			//enter will add items to cart if user presses enter after typing item, or pressing add to cart
			enter = enter || ImGui::Button("add to cart");
			
			//a way to randomly generate a unique price, only taking last items memory address if it exist
			void* p = cart.empty() ? &str : (void*)&cart[cart.size() - 1];

			//loop through cart items
			for (auto& item : cart)
			{
				//checks to see if we already added annual charge to cart
				if (strcmp("Premium account First Time Annual Charge", item.name.c_str()) == 0)
				{
					//if we found it, but don't need it, we will remove it from the cart
					if (!addPremiumPrice)
					{
						cart.erase(cart.begin());
						addPremiumPrice = false;
						goto EndStore;
					}//since we found it, we don't need to add it
					addPremiumPrice = false;
				}
				//if adding items to cart
				if (enter)
				{
					//check to see if item already is in cart
					if (strcmp(str.c_str(), item.name.c_str()) == 0)
					{
						//increases the quantity in the cart to make it more compact, and jumps to end of this window
						item.amount++;
						goto EndStore;
					}
				}
			}
			//if added item is not found, add it to cart, but keep price between 1000 pennies, or $10
			if (enter)
				cart.push_back(Items(1, str, (unsigned int)(p) % 1000));
			//if we need to add the premium charge to account, add it here
			if (addPremiumPrice)
				cart.emplace(cart.begin(), Items(1, "Premium account First Time Annual Charge", 4000));
		EndStore://move above this if statment?
		}

		//if the customer is not logged in, they will see this login screen
		else
		{
			//TODO: implement checking for user or password
			std::string pass;
			//takes in the username
			ImGui::InputText("username", &str[0], 255, ImGuiInputTextFlags_EnterReturnsTrue);
			//takes in the password, hiding the keys typed
			bool password = ImGui::InputText("password", &pass[0], 255, ImGuiInputTextFlags_Password | ImGuiInputTextFlags_EnterReturnsTrue);

			//if customer hits enter on password or hits sign in button, check credentials and login
			if (password || ImGui::Button("login"))
			{
				loggedin = true;
			}
			//looks through the cart and deletes the annual charge if present and not logged in.
			for (auto& item : cart)
			{
				std::string& str0 = item.name;
				if (strcmp("Premium account First Time Annual Charge", str0.c_str()) == 0)
				{
					//deletes the first item since we put that charge in the first spot every time
					cart.erase(cart.begin());
				}
			}

		}
		ImGui::End();

	}

	#if 1
	{
		//debug settings, allow us to alter the settings that normally we cant change easily
		ImGui::Begin("settings");

		ImGui::Checkbox("logged in status", &loggedin);
		ImGui::InputInt("Credit card Value", &tmpAccount.CC);
		ImGui::Checkbox("admin?", &tmpAccount.admin);
		ImGui::Checkbox("Premium?", &tmpAccount.Premium);
		ImGui::Checkbox("firstAnnualPurchase?", &tmpAccount.firstAnnualPurchase);
		//will keep increasing the credit card number until we hit one in the bank, overflowing when we exceed the maximum value
		bool found = false;
		while (!found)
		{
			for (auto acc : bank->accounts)
			{
				if (acc.first == tmpAccount.CC)
				{
					ImGui::Text("CC: %d, balance: @%d.%d", acc.first, acc.second / 100, acc.second % 100);
					found = true;
				}
			}
			if (!found)
				tmpAccount.CC = (tmpAccount.CC + 1) % 931;
		}
		ImGui::End();
	}
	#endif

	//begin the shopping cart UI
	ImGui::Begin("shopping Cart");
	{
		//stores the total cost in cents, bills, and total to prevent floating point errors
		int cents = 0, bills = 0, total;
		//add all the prices of the cart
		for (auto& item : cart)
		{
			auto p = item.price().substr(1);
			cents += std::stoi(p.substr(p.find(".") + 1));
			bills += std::stoi(p);
		}
		//adds the bills and pennies together
		total = cents + bills * 100;
		//sets the label to the total price
		auto label = "total: $" + std::to_string(total / 100) + "." + std::to_string(total % 100);

		//updates the credit card the banking system is looking for, and the amount to charge if checked out
		creditCard = tmpAccount.CC;
		chargingAmount = total;

		//if checking out
		if (ImGui::Button("Checkout?"))
		{
			//tell the bank we want to charge our card
			bank->Request = true;
			//wait for the bank to process our payment, when finished it will change this to false, breaking the loop
			while (bank->Request)
			{
				std::cout << "";
			}
			//since the bank only changes this value while processing, since it finished, we can look at the value
			auto code = bank->ConformationNumber;
			//if the code isn't -1, everything worked smoothly, and display conformation code, TODO: add that to imgui
			if ((code != -1))
			{
				std::cout << "Conformation Code: " << code << "\n";

				//they made a purchase, so we can remove that charge till next year
				tmpAccount.firstAnnualPurchase = false;
				//empty their cart TODO: add the cart to their order history
				cart.clear();
			}
			else
				std::cout << "an error occurred processing your payment\n";
		}

		//display the total cart price on the same line as the button
		ImGui::SameLine();
		ImGui::Text(label.c_str());

		//add a separating line to separate cart from total
		ImGui::Separator();

		//loop through the cart
		for (int i = 0; i < cart.size(); i++)
		{
			//save the item as a reference
			auto& item = cart[i];
			//check if the item is the premium charge, since we don't want them modifying this charge in their cart
			bool premiumCharge = strcmp("Premium account First Time Annual Charge", item.name.c_str()) == 0;


			//save this label, since two imgui items with the same name will trigger at the same time when activating one
			auto label = "##" + item.name + std::to_string(i + 1);
			
			//don't let them change how many of the premium charges are on the cart
			if (!premiumCharge)
				ImGui::InputInt(label.c_str(), &item.amount);
			//ensure not to get negative items
			item.amount = __max(item.amount, 0);

			//if we aren't putting the input int, we don't need the same line
			if (!premiumCharge)
				ImGui::SameLine();

			//print the item name and the price
			ImGui::Text(item.name.c_str());
			ImGui::Text(item.price().c_str());

			//don't let customers remove the premium charge
			if (!premiumCharge)
			{
				ImGui::SameLine();
				if (ImGui::Button(("remove item?##" + std::to_string(i) + item.name).c_str()))
					cart.erase(cart.begin() + i);
			}
		}
		ImGui::End();
	}

}

int main()
{
	//initialize imgui settings
	ImGuiStartup* start = new ImGuiStartup(*app);

	//add this to application, so we can deal with events
	app->imgs = start;

	//loop until we stop application
	while (app->m_Running)
	{
		//begin imgui render pass
		start->Begin();

		OnImGuiRender(*app);
		
		//render everything we did in above function
		start->End();

		//update application frames
		app->Run();
	}
	//wait for the thread to finish before exiting
	bankingSystem.join();

	//memory cleanup
	delete app;
	delete start;
}