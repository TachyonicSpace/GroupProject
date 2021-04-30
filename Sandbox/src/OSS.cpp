#include "ImGui/ImGuiLayer.h"

#include "imgui.h"
#include "iostream"
#include "Bank.h"
#include "Users.h"
#include "Item.h"
#include "invoice.h"
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
		//prevents this thread from saturating the cpu core
		std::this_thread::sleep_for(2s);
	}
	});




//renders the item
class OnImGuiRender
{
public:
	Application& app;

	//string we need to store the input text from imgui
	std::string str = "";

	//boolean to see if we need to add premium price, or verify the customers identity
	bool addPremiumPrice = false, verified = false;

	//bool to see if we are currently adding new customer, and using int so we can store if we are creating a new supplier
	int newcust = 0;
	bool purchaseCart = false;
	invoice* selectedOrder = nullptr;
	//allows us to redirect std::cout into imgui
	std::stringstream output;

	OnImGuiRender(Application& App)
		:app(App) {}

	//called every frame
	void onUpdate()
	{
		//causing std::out to direct to output instead
		std::streambuf* old = std::cout.rdbuf(output.rdbuf());

		//bool to see if we add on a charge for the premium account
		addPremiumPrice = loggedin && loggedin->PremiumAccount && loggedin->FirstAnnualPurchase;

		//show the menu at top of screen
		menu();

		//if banks is not working on an order, and the order failed(code == -1)
		if (!bank->Request && bank->ConformationNumber == -1)
		{
			//reenter the card, only thing they can do
			ImGui::Begin("re-enter card");

			//reenter the card, or modify order
			ImGui::InputInt("an error occurred, please enter a new credit card.", &creditCard);
			if (ImGui::Button("submit new card"))
			{
				makeOrder();
				std::cout << "please try again";
			}
			if (ImGui::Button("Cancel or modify order?"))
			{
				bank->ConformationNumber = 0;
			}

			ImGui::End();
		}
		//if not changing their card, do everything else
		else
		{
			store();
		}


		//restore std::cout;
		std::cout.rdbuf(old);
		//prints std::cout to console screen in our application
		ImGui::Begin("Console output/Warnings");
		ImGui::TextWrapped(output.str().c_str());

		ImGui::End();
	}

	//loads menu at top left of screen
	void menu()
	{
		//if they are logged in(pointer isnt null)
		if (loggedin)
		{
			//menu
			if (ImGui::BeginMenuBar())
			{
				//adds a menu to the top for account settings
				if (ImGui::BeginMenu("Account Settings"))
				{
					//allows them to change account status
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

					//logs out user
					if (ImGui::MenuItem("logout"))
						loggedin = false;

					//deletes account
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

	//loads the store screens, like selecting items when logged in
	void store()
	{
		//begins a page for the store UI, like logging in and looking at items offered
		ImGui::Begin("Store");
		{

			//if the customer is logged in, then they will see these options
			if (loggedin)
			{
				//suplier will see process order, ship order and view stock
				if (loggedin->supplier)
				{
					processOrder();
					shipOrder();
					viewStock();
				}
				//customer sees viewing orders, invoices(must select order) and adding items to cart
				else
				{
					viewInvoice();
					viewOrder();
					selectItems();
					shoppingCart();
				}
			}

			//if the customer is not logged in, they will see this login screen
			else if (!newcust)
				login();

			//making a new account
			else if (newcust == 1)
				createAccount();

			//making a new supplier account
			else
				createSupplierAccount();

			ImGui::End();

		}
	}

	//shows all items in cart
	void shoppingCart()
	{
		//begin the shopping loggedin->cart UI
		ImGui::Begin("shopping Cart");
		{
			//if no one is logged in, inform them they need to log in to use the cart
			if (!loggedin)
			{
				ImGui::Text("please log in");
				ImGui::End();
				return;
			}
			//add together all the prices of the loggedin->cart
			total = 0;
			for (auto& item : loggedin->cart)
			{
				auto p = item.price(loggedin->PremiumAccount).substr(1);
				total += std::stoi(p.substr(p.find(".") + 1));
				total += std::stoi(p) * 100;
			}
			//sets the label to the total price
			auto label = "total: $" + std::to_string(total / 100) + "." + std::to_string(total % 100);

			//displayes premium checkout if premium account
			std::string checkout = "Checkout?";
			if (loggedin && loggedin->PremiumAccount)
				checkout = "Premium " + checkout;

			//if loggedin user's cart isnt empty
			if (loggedin && !loggedin->cart.empty())
			{
				//allows cusotmer to check out
				if (ImGui::Button(checkout.c_str()))		//if checking out
					ImGui::OpenPopup("please choose delivery option?");

				//allows user to choose their delivery option
				if (ImGui::BeginPopup("please choose delivery option?"))
				{
					//if picking up, set checking out to true on button clicked
					purchaseCart |= ImGui::Button("in store pickup?(no additional charge)");
					//if getting delivery, add delivery charge on order and check out
					if (ImGui::Button("delivery? (with a 3$ delivery fee)"))
					{
						purchaseCart = true;
						loggedin->cart.emplace(loggedin->cart.begin(), Item("delivery charge", 1, 0, "delivery charge", 300, 300));
					}

					//if selected item, close popup menu
					if (purchaseCart)
						ImGui::CloseCurrentPopup();

					ImGui::EndPopup();
					//update credit card to users credit card
					creditCard = loggedin->CreditCardNumber;
				}
				//make order if purchusing
				if (purchaseCart)
					makeOrder();


				ImGui::SameLine();		//display the total loggedin->cart price on the same line as the button
				ImGui::Text(label.c_str());


				ImGui::Separator();		//add a separating line to separate loggedin->cart from total
			}


			for (int i = 0; i < loggedin->cart.size(); i++)		//loop through the loggedin->cart
			{
				//check if the item is the premium charge, since we don't want them modifying this charge in their loggedin->cart
				bool premiumCharge = strcmp("Premium account First Time Annual Charge", loggedin->cart[i].name.c_str()) == 0;


				//save this label, since two imgui items with the same name will trigger at the same time when activating one
				auto label = "##" + loggedin->cart[i].name + std::to_string(i + 1);

				if (!premiumCharge)			//don't let them change how many of the premium charges are on the loggedin->cart
					ImGui::InputInt(label.c_str(), &loggedin->cart[i].amount);
				loggedin->cart[i].amount = __max(loggedin->cart[i].amount, 0);			//ensure not to get negative items

				if (loggedin->cart[i].amount == 0)
				{
					loggedin->cart.erase(loggedin->cart.begin() + i);
					ImGui::End();
					return;
				}
				if (premiumCharge)			//if we aren't putting the input int, we don't need the same line
					ImGui::SameLine();

				ImGui::Text((loggedin->cart[i].name + " \n\t" + loggedin->cart[i].price(loggedin->PremiumAccount)).c_str());

			}
			ImGui::End();
		}
	}

private:

	//Customer (or supplier)
	void login()
	{
		char pass[5] = { 0 };
		//takes in the username
		ImGui::InputText("username", &str[0], 5, ImGuiInputTextFlags_EnterReturnsTrue);
		//takes in the password, hiding the keys typed
		bool password = ImGui::InputText("password", pass, 5, ImGuiInputTextFlags_Password);

		//if customer hits enter on password or hits sign in button, check credentials and login
		if (password)
		{
			for (Users& user : allUsers)
			{
				if (strcmp(str.c_str(), user.username.c_str()) == 0)
					if (strcmp(pass, user.password.c_str()) == 0)
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

		//sets newcust to 1|0 if the button is pressed, and 2 if the supplier button is pressed
		newcust = (ImGui::Button("Sign up for new account?")) ? 1 : 0;

		newcust = (ImGui::Button("Create new supplier account?")) ? 2 : newcust;

	}

	//Customer
	void createAccount()
	{
		static std::string username, password, password0, phone, address;
		static int cc = 0;
		static bool premium = false;

		//takes input text and stores it in the variable, and setting the length limit
		ImGui::InputTextWithHint("username", "please enter a new 4 digit username", &username[0], 5);
		ImGui::InputTextWithHint("password", "please enter a new 4 digit password", &password[0], 5);
		ImGui::InputTextWithHint("confirm password", "please confirm your 4 digit password", &password0[0], 5);

		//if username exist, inform customer
		if (findUsername(username))
			ImGui::TextColored({ 1, .2f, .36f, 1 }, "\tUsername already in use, please enter another one");

		//if password and confirm password dont match, inform cusotmer and dont let them move on
		else if (strcmp(password.c_str(), password0.c_str()) != 0)
		{
			ImGui::TextColored({ 1, .2f, .36f, 1 }, "\tplease make sure your passwords match");
		}
		//takes rest of customers information
		else
		{
			if (ImGui::InputTextWithHint("address", "please enter your 4 digit street number", &address[0], 5))
				address.shrink_to_fit();
			if (ImGui::InputTextWithHint("phone number", "please enter a 10 digit phone number", &phone[0], 11))
				phone.shrink_to_fit();
			ImGui::InputInt("2 digit credit card", &cc);
			ImGui::Checkbox("sign up for premium? there will be a $40 charge added on your first purchase of the year?", &premium);

			//makes sure the user information is in correct range
			if (strlen(username.c_str()) == 4 && strlen(password.c_str()) == 4 && strlen(phone.c_str()) == 10 && strlen(address.c_str()) == 4 && cc > 9 && cc < 100 && ImGui::Button("create account"))
			{
				allUsers.push_back({ username.c_str(), password.c_str(), std::vector<Item>(), false, (address + " st").c_str(), phone.c_str(), cc, (bool)premium, true });
				loggedin = &allUsers[allUsers.size() - 1];

				//resetting values to empty
				username = password = password0 = phone = address = "";
				premium = cc = 0;
				newcust = false;
			}
			//if hit cancel button, clear fields and return to login page
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
				allUsers.push_back({ username, password, {}, true, "N/A ", "0000000000", -1, false, true });
				loggedin = &allUsers[allUsers.size() - 1];

				//resetting values to empty
				username = password = password0 = "";
				newcust = false;
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

		//filter to filter items by name or description
		static ImGuiTextFilter filter;
		ImGui::Text("Filter usage:\n"
			"		         display all lines\n"
			"  xxx			 display lines containing \"xxx\"\n"
			"  xxx,yyy		 display lines containing \"xxx\" or \"yyy\"\n"
			"  -xxx		     hide lines containing \"xxx\"");
		filter.Draw();

		ImGui::Separator();

		bool first = true;
		for (auto& item : inventory)
		{
			if (!first)
				ImGui::Separator();

			if (filter.PassFilter(item.name.c_str()) || filter.PassFilter(item.description.c_str()))
			{
				ImGui::BulletText(item.name.c_str());
				ImGui::TextColored({ .2, .89, .3, 1 }, "\tPremium Price: %s", GetPrice(item.premPrice).c_str()); ImGui::SameLine();
				ImGui::TextColored({ .89, .2, .43, 1 }, "\tRegular Price: %s", GetPrice(item.regPrice).c_str());
				ImGui::TextColored({ .2, .12, .82, 1 }, "\t%s", item.description.c_str());

				if (ImGui::Button(("add to cart##" + item.name + item.description).c_str()))
				{
					enter = &item;
					break;
				}
			}
			first = true;
		}

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
			loggedin->cart.push_back({ enter->name, 1, 0, enter->description, enter->regPrice, enter->premPrice });
		//if we need to add the premium charge to account, add it here
		if (addPremiumPrice)
			loggedin->cart.emplace(loggedin->cart.begin(), Item("Premium account First Time Annual Charge", 1, 0, "a charge placed on your card \nfor being a premium member, \nallowing you to get the cheaper \nprices at checkout.", 4000, 4000));
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

			time_t rawTime;
			struct tm* timeinfo;
			time(&rawTime);
			timeinfo = localtime(&rawTime);
			std::string date;
			date += ((timeinfo->tm_mon < 10) ? "0" + std::to_string(timeinfo->tm_mon) : "" + std::to_string(timeinfo->tm_mon)) + "/";
			date += ((timeinfo->tm_mday < 10) ? "0" + std::to_string(timeinfo->tm_mday) : "" + std::to_string(timeinfo->tm_mday)) + "/";
			date += std::to_string(timeinfo->tm_year + 1900);

			allOrders.push_back({ loggedin->username, loggedin->PremiumAccount, loggedin->cart, date, total, loggedin->CreditCardNumber, status::Ordered, bank->ConformationNumber });

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
		ImGui::Begin("Show Order");
		{
			static char* showingStatus[4] = { "all", "processing", "Shipped", "ready for pickup" };
			static std::string currentShowingStatus = showingStatus[0];
			static status* statusFilter = nullptr;

			static invoice* currentOrder = nullptr;
			ImGui::Text((" " + loggedin->username + "'s order history").c_str());
			ImGui::Text(("show ")); ImGui::SameLine();


			if (ImGui::BeginCombo("##order status's", currentShowingStatus.c_str()))
			{
				for (int i = 0; i < 4; i++)
				{
					bool isSelected = currentShowingStatus == showingStatus[i];
					if (ImGui::Selectable(showingStatus[i], isSelected))
					{
						currentShowingStatus = showingStatus[i];
						if (i != 0)
						{
							statusFilter = new status();
							*statusFilter = (status)(i - 1);
						}
						else
						{
							delete(statusFilter);
							statusFilter = nullptr;
						}
					}

					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}


			ImGui::SameLine();
			ImGui::Text(" orders"); ImGui::SameLine();


			ImGui::Separator();
			int numOrders = 0;
			for (auto& order : allOrders)
			{
				if (strcmp(order.user.c_str(), loggedin->username.c_str()) == 0 && (!statusFilter || *statusFilter == order.currentStatus))
				{
					ImGui::Separator();
					numOrders += 1;
					std::string str = "order number #" + std::to_string(order.conformationCode) + "\n{\n";
					for (auto item : order.cart)
					{
						str += "\tyou ordered: " + std::to_string(item.amount) + " - " + item.name + " @ " + GetPrice((order.premium) ? item.premPrice : item.regPrice) + "\n";
					}
					str += "}\nstatus: ";
					switch (order.currentStatus)
					{
					case status::Ordered:
						str += "Ordered, In Processing";
						break;
					case status::Ready:
						str += "Ready";
						break;
					case status::Shipped:
						str += "Shipped";
						break;
					case status::Pickup:
						str += "Ready for Pickup";
						break;
					}
					bool selected = false;
					ImGui::Selectable(str.c_str(), &selected);
					if (selected)
					{
						selectedOrder = &order;
						verified = false;
					}
				}
			}
			if (numOrders == 0)
				ImGui::Text("sorry, we don't have any orders for you, but when you do make an order, it will appear here.");

			ImGui::End();
		}
	}

	//customer
	void viewInvoice()
	{
		ImGui::Begin("Invoice");
		if (selectedOrder && strcmp(selectedOrder->user.c_str(), loggedin->username.c_str()) == 0)
		{
			ImGui::Text(("order #" + std::to_string(selectedOrder->conformationCode) + " details").c_str());

			ImGui::Separator();


			std::string str = selectedOrder->date + "\nCard number: " + std::to_string(selectedOrder->card) + "\nTotal: " + GetPrice(selectedOrder->total) + "\n{\n";
			for (auto item : selectedOrder->cart)
			{
				str += "\tyou ordered: " + std::to_string(item.amount) + " - " + item.name + " @ " + GetPrice((selectedOrder->premium) ? item.premPrice : item.regPrice) + "\n";
			}
			str += "}\nstatus: ";
			switch (selectedOrder->currentStatus)
			{
			case status::Ordered:
				str += "Ordered, In Processing";
				break;
			case status::Ready:
				str += "Ready";
				break;
			case status::Shipped:
				str += "Shipped";
				break;
			case status::Pickup:
				str += "Ready for Pickup";
				break;
			}

			ImGui::TextWrapped(str.c_str());
			ImGui::Separator();
			if (ImGui::Button("clear order details?"))
				selectedOrder = nullptr;
		}
		ImGui::End();

	}



	//supplier
	void processOrder()
	{
		ImGui::Begin("Process orders");

		Item* enter = nullptr;

		static invoice* currentOrder = nullptr;

		for (auto& order : allOrders) {
			if (order.currentStatus == status::Ordered) {

				std::string str = "order number #" + std::to_string(order.conformationCode) + "\n{\n";
				for (auto item : order.cart)
				{
					str += "\tyou ordered: " + std::to_string(item.amount) + " - " + item.name + " @ " + GetPrice((order.premium) ? item.premPrice : item.regPrice) + "\n";
				}
				str += "}\nstatus: ordered";

				if (ImGui::Selectable(str.c_str())) {
					std::string str = "order number #" + std::to_string(order.conformationCode) + "\n{\n";

					for (auto& item : order.cart) {
						for (auto& iItem : inventory) {
							if (strcmp(item.name.c_str(), iItem.name.c_str()) == 0) {
								iItem.amount -= item.amount;
								iItem.reserved += item.amount;
							}
						}

					}
					std::cout << "Order has been made.";

					order.currentStatus = status::Ready;
				}
			}
		}
		ImGui::End();

	}

	//supplier
	void shipOrder()
	{
		ImGui::Begin("ship orders");

		static char* showingStatus[4] = { "all", "Ready", "Shipped", "ready for pickup" };
		static std::string currentShowingStatus = showingStatus[0];
		static status* statusFilter = nullptr;

		static invoice* currentOrder = nullptr;


		ImGui::SameLine();
		ImGui::Text(" orders");


		ImGui::Separator();
		int numOrders = 0;
		for (auto& order : allOrders)
		{
			bool selected = false;

			if (!statusFilter || *statusFilter == order.currentStatus)
			{
				numOrders += 1;
				std::string str = "order number #" + std::to_string(order.conformationCode) + "\n{\n";
				for (auto item : order.cart)
				{
					str += "\tordered: " + std::to_string(item.amount) + " - " + item.name + " @ " + GetPrice((order.premium) ? item.premPrice : item.regPrice) + "\n";
				}
				str += "}";
				ImGui::Text(str.c_str());
				str = "order Status: ";
				switch (order.currentStatus)
				{
				case status::Ordered:
					break;
				case status::Ready:
					str += "Ready";

					if (ImGui::Button("Process order and set aside stock?"))
					{
						if (strcmp(order.cart[0].name.c_str(), "delivery charge") == 0)
						{
							order.currentStatus = status::Shipped;
						}
						else
						{
							order.currentStatus = status::Pickup;
						}
						if (order.currentStatus != status::Ordered && order.currentStatus != status::Ready)
						{
							for (auto& items : inventory)
							{
								for (auto& myItems : order.cart)
								{
									if (strcmp(items.name.c_str(), myItems.name.c_str()) == 0)
									{
										items.reserved += myItems.amount;
										items.amount -= myItems.amount;
									}
								}
							}
						}
					}



					break;
				case status::Shipped:
					str += "Shipped";
					ImGui::Text(str.c_str());
					break;
				case status::Pickup:
					str += "Ready for Pickup";
					ImGui::Text(str.c_str());
					break;
				}
				ImGui::Separator();
			}
		}
		if (numOrders == 0)
			ImGui::Text(("sorry, we don't have any " + currentShowingStatus + " orders for you, but they will appear here.").c_str());

		ImGui::End();
	}

	//supplier
	void viewStock()
	{

		ImGui::Separator();
		for (auto& item : inventory)
		{
			bool first = true;
			if (!first)
				ImGui::Separator();

			if (first == true)
			{
				ImGui::Text(item.name.c_str());
				ImGui::TextColored({ .2, .89, .3, 1 }, "\tPremium Price: %s", GetPrice(item.premPrice).c_str());
				ImGui::TextColored({ .89, .2, .43, 1 }, "\tRegular Price: %s", GetPrice(item.regPrice).c_str());
				ImGui::TextColored({ .2, .12, .82, 1 }, "\t%s", item.description.c_str());
				ImGui::InputInt(("Available Stock##" + item.name).c_str(), &item.amount);
				ImGui::TextColored({ .87, .50, .40, 1}, "\tReserved Stock: %d", item.reserved);
				ImGui::TextColored({ .87, .50, .40, 1}, "\ttotal stock: %d", item.amount + item.reserved);

				ImGui::Separator();
			}
			first = true;
		}
	}

};

int main()
{
	ImGuiStartup* start = new ImGuiStartup(*app);	//initialize imgui settings
	OnImGuiRender OOSS(*app);

	GetInventory();

	bank->cf = GetAllOrders();
	getUsers();

	while (app->m_Running)	//loop until we stop application

	{
		if (!loggedin)
			ImGui::StyleColorsDark();
		else if (!loggedin->PremiumAccount)
			ImGui::StyleColorsDark();
		else if (loggedin->supplier)
			ImGui::StyleColorsLight();
		else
			ImGui::StyleColorsClassic();

		start->Begin();		//begin imgui render pass
		OOSS.onUpdate();
		start->End();		//render everything we did in above function
	}
	bankingSystem.join();	//wait for the thread to finish before exiting

	setUsers();
	SetAllOrders();
	SetInventory();

	//memory cleanup
	delete app;
	delete start;
	bank->~Bank();
}