#include "ImGui/ImGuiLayer.h"

#include "imgui.h"
#include "iostream"

class Items
{
public:
	Items(int Amount, std::string Name, int Price)
		:amount(Amount), name(Name), priceInPennies(Price){}
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

std::vector<Items> cart;

void OnImGuiRender(Application& app)
{
	{
		ImGui::Begin("Store");

		static std::string str = "hello";
		bool enter;
		enter = ImGui::InputText("hello", &str[0], 255, ImGuiInputTextFlags_EnterReturnsTrue);
		ImGui::SameLine();

		if (enter || ImGui::Button("add to cart"))
		{
			void* p = cart.empty() ? &str : (void*)&cart[cart.size()-1];
			cart.push_back(Items(1, str, (unsigned int)(p)%1000 ));
		}

		ImGui::End();
	}

	ImGui::Begin("shopping Cart");
	int cents = 0, bills = 0, total;
	for (auto& item : cart)
	{
		auto p = item.price().substr(1);
		cents += std::stoi(p.substr(p.find(".")+1));
		bills += std::stoi(p);
	}
	total = cents + bills * 100;
	auto label = "Checkout? total: $" + std::to_string(total / 100) + "." + std::to_string(total % 100);
	static bool checkout = false;
	ImGui::Checkbox(label.c_str(), &checkout);
	ImGui::Separator();

	for (int i = 0; i < cart.size(); i++)
	{
		auto& item = cart[i];
		auto label = "##" + item.name + std::to_string(i+1);
		ImGui::InputInt(label.c_str(), &item.amount);
		item.amount = __max(item.amount, 0);
		
		ImGui::SameLine();
		ImGui::Text(item.name.c_str());

		ImGui::Text(item.price().c_str());
		ImGui::SameLine();
		if (ImGui::Button(("remove item?##" + std::to_string(i) + item.name).c_str()))
			cart.erase(cart.begin() + i);
	}

	ImGui::End();
}


int main()
{
	Application* app = new Application();
	ImGuiStartup* start = new ImGuiStartup(*app);
	app->imgs = start;
	while (app->m_Running)
	{
		start->Begin();
		OnImGuiRender(*app);
		start->End();
		app->Run();
	}

	delete app;
	delete start;
}