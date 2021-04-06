#include "ImGui/ImGuiLayer.h"

#include "imgui.h"
#include "iostream"
#include "ProjectClassFiles/Bank/Bank.h"
#include "ProjectClassFiles/Users/Users.h"
#include "chrono"
#include <thread>
using namespace std::chrono_literals;

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



//TODO: implement in customer's account
std::vector<Items> cart;
//TODO: replace with a user ptr
Users* loggedin = nullptr;

//store the account information the banking system needs
int creditCard = 121, chargingAmount = 1000;//used by banking system
Bank* bank = new Bank();
Application* app = new Application();//needed to see when program ends


//this thread will run separate of the main thread until the lambda function stops, so when the program stops
std::thread bankingSystem([&] {
	while (app->m_Running)
	{
		//if the bank has a request, charge the amount to the card, else wait
		if (bank->Request)
			bank->ChargeAmount(creditCard, chargingAmount);
	}
	});




//renders the item
class OnImGuiRender
{
public:
	Application& app;
	#define debug

	OnImGuiRender(Application& App)
		:app(App) {}

	void onUpdate()
	{
		menu();
		store();
		shoppingCart();

		#ifdef debug
		debugSettings();
		#endif
	}

	void menu()
	{
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
					if (loggedin->IsAdmin)
						ImGui::MenuItem("Browse Catalog");

					if (ImGui::MenuItem("logout"))
						loggedin = false;
				}
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

	}

	void store()
	{
		//begins a page for the store UI, like logging in and looking at items offered
		ImGui::Begin("Store");
		{
			//string we need to store the input text from imgui
			static std::string str = "";
			//bool to see if we add on a charge for the premium account
			bool addPremiumPrice = loggedin && loggedin->PremiumAccount && loggedin->FirstAnnualPurchase;

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
					for(Users& user : allUsers)
					{
						if (strcmp(str.c_str(), user.username.c_str()) == 0)
							if (strcmp(pass.c_str(), user.password.c_str()) == 0)
								loggedin = &user;
					}
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

		EndStore://move above this if statement?
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

			if (ImGui::Button("Checkout?"))		//if checking out
			{
				creditCard = loggedin->CreditCardNumber;
				chargingAmount = total;

				bank->Request = true;			//tell the bank we want to charge our card
				std::cout << "\n";
				int i = 0;
				while (bank->Request)			//wait for the bank to process our payment, when finished it will change this to false, breaking the loop
				{
					switch (i++)
					{
					case 0:
						std::cout << "\rWaiting for Response /";
						std::this_thread::sleep_for(1s);
						break;
					case 1:
						std::cout << "\rWaiting for Response |";
						std::this_thread::sleep_for(1s);
						break;
					case 2:
						std::cout << "\rWaiting for Response \\";
						std::this_thread::sleep_for(1s);
						break;
					case 3:
						std::cout << "\rWaiting for Response -";
						std::this_thread::sleep_for(1s);
						i %= 4;
						break;
					}
				}
				//since the bank only changes this value while processing, since it finished, we can look at the value
				auto code = bank->ConformationNumber;
				//if the code isn't -1, everything worked smoothly, and display conformation code, TODO: add that to imgui
				if ((code != -1))
				{
					std::cout << "\rConformation Code: " << code << "\n";

					//they made a purchase, so we can remove that charge till next year
					loggedin->FirstAnnualPurchase = false;
					//empty their cart TODO: add the cart to their order history
					cart.clear();
				}
				else
					std::cout << "an error occurred processing your payment\n";
			}

			ImGui::SameLine();		//display the total cart price on the same line as the button
			ImGui::Text(label.c_str());


			ImGui::Separator();		//add a separating line to separate cart from total

			for (int i = 0; i < cart.size(); i++)		//loop through the cart
			{
				//check if the item is the premium charge, since we don't want them modifying this charge in their cart
				bool premiumCharge = strcmp("Premium account First Time Annual Charge", cart[i].name.c_str()) == 0;


				//save this label, since two imgui items with the same name will trigger at the same time when activating one
				auto label = "##" + cart[i].name + std::to_string(i + 1);

				if (!premiumCharge)			//don't let them change how many of the premium charges are on the cart
					ImGui::InputInt(label.c_str(), &cart[i].amount);
				cart[i].amount = __max(cart[i].amount, 0);			//ensure not to get negative items


				if (!premiumCharge)			//if we aren't putting the input int, we don't need the same line
					ImGui::SameLine();

				ImGui::Text(cart[i].name.c_str());			//print the item name and the price
				ImGui::Text(cart[i].price().c_str());

				if (!premiumCharge)			//don't let customers remove the premium charge
				{
					ImGui::SameLine();
					if (ImGui::Button(("remove item?##" + std::to_string(i) + cart[i].name).c_str()))
						cart.erase(cart.begin() + i);
				}
			}
			ImGui::End();
		}
	}

};

int main()
{
	ImGuiStartup* start = new ImGuiStartup(*app);	//initialize imgui settings
	OnImGuiRender OOSS(*app);
	getUsers();

	while (app->m_Running)	//loop until we stop application

	{
		start->Begin();		//begin imgui render pass
		OOSS.onUpdate();
		start->End();		//render everything we did in above function
	}
	bankingSystem.join();	//wait for the thread to finish before exiting

	//memory cleanup
	delete app;
	delete start;

	//keeps console up if we want to look at it.
	std::this_thread::sleep_for(2min);
}