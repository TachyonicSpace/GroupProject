#include "ImGui/ImGuiLayer.h"

#include "imgui.h"
#include "iostream"
#include "ProjectClassFiles/Bank/Bank.h"
#include "ProjectClassFiles/Users/Users.h"
#include "ProjectClassFiles/Items/Item.h"
#include "chrono"
#include <thread>
using namespace std::chrono_literals;



//a pointer to store the current user.
Users* loggedin = nullptr;

//store the account information the banking system needs
int creditCard = 121, total = 0;//used by banking system
Bank* bank = new Bank();
Application* app = new Application();//needed to see when program ends


//this thread will run separate of the main thread until the lambda function stops, so when the program stops
std::thread bankingSystem([&] {
	while (app->m_Running)
	{
		//if the bank has a request, charge the amount to the card, else wait
		if (bank->Request)
			bank->ChargeAmount(creditCard, total);
	}
	});




//renders the item
class OnImGuiRender
{
public:
	Application& app;

	//string we need to store the input text from imgui
	std::string str = "";
	//bool to see if we add on a charge for the premium account
	bool addPremiumPrice = loggedin && loggedin->PremiumAccount && loggedin->FirstAnnualPurchase;

	//bool to see if we are currently adding new customer
	bool newcust = false, purchaseCart = false;
	std::stringstream output;

	#define debug

	OnImGuiRender(Application& App)
		:app(App) {}

	void onUpdate()
	{
		//causing std::out to direct to output instead
		std::streambuf* old = std::cout.rdbuf(output.rdbuf());


		menu();

		if (!bank->Request && bank->ConformationNumber == -1)
		{
			ImGui::Begin("re-enter card");

			ImGui::InputInt("an error occurred, please enter a new credit card.", &creditCard);
			if (ImGui::Button("submit new card"))
			{
				makeOrder();
			}
			if (ImGui::Button("Cancel or modify order?"))
			{
				bank->ConformationNumber = 0;
			}

			ImGui::End();
		}
		else
		{
			store();
			shoppingCart();

			#ifdef debug
			debugSettings();
			bool b = true;
			ImGui::ShowDemoWindow(&b);
			#endif
		}


		//restore std::cout;
		std::cout.rdbuf(old);
		ImGui::Begin("Console output/Warnings");
		ImGui::Text(output.str().c_str());

		ImGui::End();
	}

	void menu()
	{
		if (loggedin)
		{
			//menu
			if (ImGui::BeginMenuBar())
			{
				//adds a menu to the top for account settings
				if (ImGui::BeginMenu("Account Settings"))
				{

					if (loggedin->IsAdmin)
						ImGui::MenuItem("Browse inventory");

					if (!loggedin->PremiumAccount)
					{
						if (ImGui::MenuItem("upgrade to premium?"))
							loggedin->PremiumAccount = loggedin->FirstAnnualPurchase = true;
					}
					else
					{
						if (ImGui::MenuItem("Cancel premium?"))
							loggedin->PremiumAccount = false;
					}


					if (ImGui::MenuItem("logout"))
						loggedin = false;

					if (ImGui::MenuItem("delete account"))
					{
						int i;
						for (i = 0; i < allUsers.size(); i++)
							if (&allUsers[i] == loggedin)
								break;
						allUsers.erase(allUsers.begin() + i);
						loggedin = nullptr;
					}
					ImGui::EndMenu();
				}

				ImGui::EndMenuBar();
			}
		}

	}

	void store()
	{
		//begins a page for the store UI, like logging in and looking at items offered
		ImGui::Begin("Store");
		{

			//if the customer is logged in, then they will see these options
			if (loggedin)
				selectItems();

			//if the customer is not logged in, they will see this login screen
			else if (!newcust)
				login();

			//making a new account
			else
				createAccount();

			ImGui::End();

		}
	}

	void debugSettings()
	{
		//debug settings, allow us to alter the settings that normally we cant change easily
		ImGui::Begin("settings");

		{//everything between these brackets calculate the index into all users we are
			int i = -1;
			for (auto& user : allUsers)
			{
				i++;
				if (loggedin == &user)
				{
					i--;
					break;
				}
			}
			i++;
			if (i == allUsers.size())
				i = -1;
			if (ImGui::InputInt("logged in: ", &i))
			{
				if (i < -1)
				{
					i = allUsers.size() - 1;
					loggedin = &allUsers[i];
				}
				else if (i == -1)
					loggedin = nullptr;
				else if (i >= allUsers.size())
				{
					i = -1;
					loggedin = nullptr;
				}
				else
					loggedin = &allUsers[i];

			}
		}
		if (loggedin)
		{
			ImGui::Text("username: %s\npassword: %s\nphone: %s\naddress: %s", loggedin->username.c_str(), loggedin->password.c_str(), loggedin->phone.c_str(), loggedin->address.c_str());
			ImGui::InputInt("Credit card Value", &loggedin->CreditCardNumber);
			ImGui::Checkbox("admin?", &loggedin->IsAdmin);
			ImGui::Checkbox("Premium?", &loggedin->PremiumAccount);
			ImGui::Checkbox("firstAnnualPurchase?", &loggedin->FirstAnnualPurchase);
		}
		//will keep increasing the credit card number until we hit one in the bank, overflowing when we exceed the maximum value
		bool found = false || !loggedin;
		while (!found)
		{
			for (auto acc : bank->accounts)
			{
				if (acc.first == loggedin->CreditCardNumber)
				{
					ImGui::Text("CC: %d, balance: @%d.%d", acc.first, acc.second / 100, acc.second % 100);
					found = true;
				}
			}
			if (!found)
				loggedin->CreditCardNumber = (loggedin->CreditCardNumber + 1) % 33;
		}
		ImGui::End();
	}

	void shoppingCart()
	{
		//begin the shopping loggedin->cart UI
		ImGui::Begin("shopping Cart");
		{
			//if no one is logged in, inform them they need to log in to use the loggedin->cart
			if (!loggedin)
			{
				ImGui::Text("please log in");
				ImGui::End();
				return;
			}
			//stores the total cost in cents, bills, and total to prevent floating point errors
			int cents = 0, bills = 0;
			//add all the prices of the loggedin->cart
			for (auto& item : loggedin->cart)
			{
				auto p = item.price(loggedin->PremiumAccount).substr(1);
				cents += std::stoi(p.substr(p.find(".") + 1));
				bills += std::stoi(p);
			}
			//adds the bills and pennies together
			total = cents + bills * 100;
			//sets the label to the total price
			auto label = "total: $" + std::to_string(total / 100) + "." + std::to_string(total % 100);

			//updates the credit card the banking system is looking for, and the amount to charge if checked out
			std::string checkout = "Checkout?";
			if (loggedin && loggedin->PremiumAccount)
				checkout = "Premium " + checkout;

			if (ImGui::Button(checkout.c_str()))		//if checking out
				ImGui::OpenPopup("please choose delivery option?");

			if (ImGui::BeginPopup("please choose delivery option?"))
			{

				purchaseCart |= ImGui::Button("in store pickup?(no additional charge)");
				if (ImGui::Button("delivery? (with a 3$ delivery fee)"))
				{
					purchaseCart = true;
					loggedin->cart.push_back({ "delivery charge", 1, "delivery charge", 300, 300 });
				}

				if (purchaseCart)
					ImGui::CloseCurrentPopup();

				ImGui::EndPopup();
				creditCard = loggedin->CreditCardNumber;
			}
			if (purchaseCart)
				makeOrder();


			ImGui::SameLine();		//display the total loggedin->cart price on the same line as the button
			ImGui::Text(label.c_str());


			ImGui::Separator();		//add a separating line to separate loggedin->cart from total

			for (int i = 0; i < loggedin->cart.size(); i++)		//loop through the loggedin->cart
			{
				//check if the item is the premium charge, since we don't want them modifying this charge in their loggedin->cart
				bool premiumCharge = strcmp("Premium account First Time Annual Charge", loggedin->cart[i].name.c_str()) == 0;


				//save this label, since two imgui items with the same name will trigger at the same time when activating one
				auto label = "##" + loggedin->cart[i].name + std::to_string(i + 1);

				if (!premiumCharge)			//don't let them change how many of the premium charges are on the loggedin->cart
					ImGui::InputInt(label.c_str(), &loggedin->cart[i].amount);
				loggedin->cart[i].amount = __max(loggedin->cart[i].amount, 0);			//ensure not to get negative items


				if (!premiumCharge)			//if we aren't putting the input int, we don't need the same line
					ImGui::SameLine();

				ImGui::Text(loggedin->cart[i].name.c_str());			//print the item name and the price
				ImGui::Text(loggedin->cart[i].price(loggedin->PremiumAccount).c_str());

				if (!premiumCharge)			//don't let customers remove the premium charge
				{
					ImGui::SameLine();
					if (ImGui::Button(("remove item?##" + std::to_string(i) + loggedin->cart[i].name).c_str()))
						loggedin->cart.erase(loggedin->cart.begin() + i);
				}
			}
			ImGui::End();
		}
	}

private:

	//Customer (or supplier)
	void login()
	{
		std::string pass;
		//takes in the username
		ImGui::InputText("username", &str[0], 255, ImGuiInputTextFlags_EnterReturnsTrue);
		//takes in the password, hiding the keys typed
		bool password = ImGui::InputText("password", &pass[0], 255, ImGuiInputTextFlags_Password | ImGuiInputTextFlags_EnterReturnsTrue);

		//if customer hits enter on password or hits sign in button, check credentials and login
		if (password || ImGui::Button("login"))
		{
			for (Users& user : allUsers)
			{
				if (strcmp(str.c_str(), user.username.c_str()) == 0)
					if (strcmp(pass.c_str(), user.password.c_str()) == 0)
						loggedin = &user;
			}
		}
		//looks through the loggedin->cart and deletes the annual charge if present and not logged in.
		if (loggedin)
		{
			for (auto& item : loggedin->cart)
			{
				std::string& str0 = item.name;
				if (strcmp("Premium account First Time Annual Charge", str0.c_str()) == 0)
				{
					//deletes the first item since we put that charge in the first spot every time
					loggedin->cart.erase(loggedin->cart.begin());
				}
			}
		}

		newcust = ImGui::Button("Sign up for new account?");

	}

	//Customer
	void createAccount()
	{
		static std::string username, password, password0, phone, address;
		static int cc = 0;
		static bool premium = false;

		ImGui::InputTextWithHint("username", "please enter a new 4 digit username", &username[0], 5);
		ImGui::InputTextWithHint("password", "please enter a new 4 digit password", &password[0], 5);
		ImGui::InputTextWithHint("confirm password", "please confirm your 4 digit password", &password0[0], 5);
		if (findUsername(username))
			ImGui::TextColored({ 1, .2f, .36f, 1 }, "\tUsername already in use, please enter another one");
		else if (strcmp(password.c_str(), password0.c_str()) != 0)
		{
			ImGui::TextColored({ 1, .2f, .36f, 1 }, "\tplease make sure your passwords match");
		}
		else
		{
			if (ImGui::InputTextWithHint("address", "please enter your 4 digit street number", &address[0], 5))
				address.shrink_to_fit();
			if (ImGui::InputTextWithHint("phone number", "please enter a 10 digit phone number", &phone[0], 11))
				phone.shrink_to_fit();
			ImGui::InputInt("2 digit credit card", &cc);
			ImGui::Checkbox("sign up for premium? there will be a $40 charge added on your first purchase of the year?", &premium);

			if (strlen(username.c_str()) == 4 && strlen(password.c_str()) == 4 && strlen(phone.c_str()) == 10 && strlen(address.c_str()) == 4 && cc > 9 && cc < 100 && ImGui::Button("create account"))
			{
				allUsers.push_back({ username, password, {}, newcust = false, address, phone, cc, premium, true });
				loggedin = &allUsers[allUsers.size() - 1];

				//resetting values to empty
				username = password = password0 = phone = address = "";
				premium = cc = 0;
			}
			if (ImGui::Button("cancel"))
			{
				//resetting values to empty
				username = password = password0 = phone = address = "";
				premium = cc = 0;

				newcust = false;
			}
		}
	}

	//supplier
	void createSupplierAccount()
	{
		static std::string username, password, password0;

		ImGui::InputTextWithHint("username", "please enter a new 4 digit username", &username[0], 5);
		ImGui::InputTextWithHint("password", "please enter a new 4 digit password", &password[0], 5);
		ImGui::InputTextWithHint("confirm password", "please confirm your 4 digit password", &password0[0], 5);
		if (findUsername(username))
			ImGui::TextColored({ 1, .2f, .36f, 1 }, "\tUsername already in use, please enter another one");
		else if (strcmp(password.c_str(), password0.c_str()) != 0)
		{
			ImGui::TextColored({ 1, .2f, .36f, 1 }, "\tplease make sure your passwords match");
		}
		else
		{
			if (strlen(username.c_str()) == 4 && strlen(password.c_str()) == 4 && ImGui::Button("create account"))
			{
				allUsers.push_back({ username, password, {}, newcust = false, "N/A ", "0000000000", -1, false, true });
				loggedin = &allUsers[allUsers.size() - 1];

				//resetting values to empty
				username = password = password0 = "";
			}
			if (ImGui::Button("cancel"))
			{
				//resetting values to empty
				username = password = password0 = "";

				newcust = false;
			}
		}
	}

	//customer
	void selectItems()
	{
		//pointer to see what item we are adding an item to the loggedin->cart
		Item* enter = nullptr;

		static int priceRange[] = { 0, 0xfffffff };

		//filter to filter items by name or discription
		ImGui::InputText("filter item name & discription", &str[0], 255);
		ImGui::InputInt2("min and max price filter", priceRange);
		ImGui::Separator();

		bool first = true;
		for (auto& item : inventory)
		{
			if (item.regPrice > priceRange[0] && item.regPrice < priceRange[1] && (item.name.find(str.c_str()) != std::string::npos || item.discription.find(str.c_str()) != std::string::npos))
			{
				if (!first)
					ImGui::Separator();

				ImGui::Text("%s\t\nPremium Price: %s, regular price: %s\n\t%s", item.name.c_str(), item.getPremPrice().c_str(),
					item.getRegPrice().c_str(), item.discription.c_str());

				if (ImGui::Button(("add to cart##" + item.name + item.discription).c_str()))
				{
					enter = &item;
					break;
				}
			}
		}


		//a way to randomly generate a unique price, only taking last items memory address if it exist
		void* p = loggedin->cart.empty() ? &str : (void*)&loggedin->cart[loggedin->cart.size() - 1];

		//loop through loggedin->cart items
		for (auto& item : loggedin->cart)
		{
			//checks to see if we already added annual charge to loggedin->cart
			if (strcmp("Premium account First Time Annual Charge", item.name.c_str()) == 0)
			{
				//if we found it, but don't need it, we will remove it from the cart
				if (!addPremiumPrice)
				{
					loggedin->cart.erase(loggedin->cart.begin());
					addPremiumPrice = false;
					return;
				}//since we found it, we don't need to add it
				addPremiumPrice = false;
			}
			//if adding items to loggedin->cart
			if (enter)
			{
				//check to see if item already is in loggedin->cart
				if (strcmp(enter->name.c_str(), item.name.c_str()) == 0)
				{
					//increases the quantity in the cart to make it more compact, and jumps to end of this window
					item.amount++;
					return;
				}
			}
		}
		//if added item is not found, add it to loggedin->cart, but keep price between 1000 pennies, or $10
		if (enter)
			loggedin->cart.push_back(*enter);
		//if we need to add the premium charge to account, add it here
		if (addPremiumPrice)
			loggedin->cart.emplace(loggedin->cart.begin(), Item("Premium account First Time Annual Charge", 1, "a charge placed on your card \nfor being a premium member, \nallowing you to get the cheaper \nprices at checkout.", 4000, 4000));
	}

	//customer, bank
	void makeOrder()
	{
		//clear the output screen
		output.str(std::string());

		bank->Request = true;			//tell the bank we want to charge our card
		purchaseCart = false;
		int i = 0;
		while (bank->Request)			//wait for the bank to process our payment, when finished it will change this to false, breaking the loop
		{

		}
		//since the bank only changes this value while processing, since it finished, we can look at the value
		auto code = bank->ConformationNumber;
		//if the code isn't -1, everything worked smoothly, and display conformation code, TODO: add that to imgui
		if ((code != -1))
		{
			std::cout << "Your Conformation Code is: " << code << "\n\tuse this when looking up order history\n\n";

			//they made a purchase, so we can remove that charge till next year
			loggedin->FirstAnnualPurchase = false;
			//empty their loggedin->cart TODO: add the loggedin->cart to their order history
			loggedin->cart.clear();
		}
		else
		{
			std::cout << "an error occurred processing your payment\n\n";
		}
	}

	//customer
	void viewOrder()
	{
		//TODO
		ImGui::Begin("Show Order");
		{
			ImGui::End();
		}
	}

	//customer
	void viewInvoice()
	{
		//TODO:
		ImGui::Begin("Invoices");
		{
			ImGui::End();
		}
	}

	//supplier
	void processOrder()
	{

	}

	//supplier
	void shipOrder()
	{

	}

	//supplier
	void viewStock()
	{

	}

};

int main()
{
	ImGuiStartup* start = new ImGuiStartup(*app);	//initialize imgui settings
	OnImGuiRender OOSS(*app);

	inventory.push_back(Item("tmp", 1, "temporary", 1000, 100));


	getUsers();

	while (app->m_Running)	//loop until we stop application

	{
		start->Begin();		//begin imgui render pass
		OOSS.onUpdate();
		start->End();		//render everything we did in above function
	}
	bankingSystem.join();	//wait for the thread to finish before exiting

	setUsers();

	//memory cleanup
	delete app;
	delete start;
}

/*TODO

Store order, stock, catalog in files

create supplier account is only takes user and password

store all purchase in a file, regardless of account,
			map with conformation number as key?

allow customer to view all their orders and order status

view invoice, with date, items, amount and payment info

process order (manually from supplier(admin))
ship order (manually from supplier(admin))


*/