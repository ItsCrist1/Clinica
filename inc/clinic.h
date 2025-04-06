#pragma once
#include "serializer.h"

class Clinic {
    const std::vector<User> DefaultDoctors {
        User(L"DrSmith", "#Password123", Type::GeneralPractice),
        User(L"DrJohnson", "@securE456", Type::Cardiology),
        User(L"DrWilliams", "^Doctor789", Type::Pediatrics),
        User(L"DrBrown", ")mediC2023", Type::Neurology),
        User(L"DrDavis", "(Health2024", Type::Orthopedics)
    };

    const std::vector<User> DefaultPatients {
        User(L"EmilyClark", "Se@ure123Pass"),
        User(L"JacobMiller", "P@ssw0rdSafe"),
        User(L"SophiaBrown", "H3alth#Care2023"),
        User(L"NoahWilson", "Patient$789Abcd"),
        User(L"OliviaJones", "M3dical!Records")
    };

    std::vector<std::shared_ptr<Appointment>> DefaultAppointments {
        std::make_shared<Appointment>(Date(12, 1, 2025), 2, 1),
        std::make_shared<Appointment>(Date(23, 3, 2025), 0, 3),
        std::make_shared<Appointment>(Date(15, 7, 2025), 4, 0),
        std::make_shared<Appointment>(Date(8, 12, 2025), 1, 4),
        std::make_shared<Appointment>(Date(19, 2, 2025), 3, 2)
    };

    static const u32 CurrentYear = 2025;

    static const u32 MinimumUsernameLength = 5;
    static const u32 MaximumUsernameLength = 20;

    static const u32 MinimumPasswordLength = 7;
    static const u32 MaximumPasswordLength = 50;

    const std::wstring ErrorColor = getCol({ 255, 0, 0 });
    const std::wstring SelectedColor = getCol({ 245, 212, 66 });
    const std::wstring UnselectedColor = getCol({ 112, 109, 96 });

    const Serializer serializer;
    std::shared_ptr<User> CurrentUser;
    std::vector<std::shared_ptr<Appointment>> CurrentAppointments;
    u32 CurrentIdx;

    std::vector<User> doctors, patients;
    std::vector<std::shared_ptr<Appointment>> appointments;

	void saveData() const;
	void initializeData();
	void fetchAppointments(const bool);
	void modifyDate(Date&) const;
	std::pair<std::shared_ptr<User>, u32> pickUser(const bool, const Date& date = Date::Default) const;
	void createAppointment();
	void deleteAppointment(const u8);
	void mainServiceMenu(const bool);
	std::pair<std::shared_ptr<User>, u32> isValidName(const std::wstring&) const;
	bool showPasswordError(const bool, const std::wstring&) const;
	void execPatientMenu(const bool);

public:
    Clinic(const std::string&);
    void MainMenu();
};