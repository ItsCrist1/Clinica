#pragma once
#include "data.h"
#include <vector>

class Serializer {
	const std::string SaveFile;

	void saveDate(std::ofstream&, const Date&) const;
	void savePatient(std::ofstream&, const User&) const;
	void saveDoctor(std::ofstream&, const User&) const;
	Date loadDate(std::ifstream&) const;
	User loadDoctor(std::ifstream&) const;
	User loadPatient(std::ifstream&) const;
	void loadAppointments(std::ifstream&, std::vector<std::shared_ptr<Appointment>>&) const;

public:
	Serializer(const std::string&);
	void SaveData(const std::vector<User>&, const std::vector<User>&, const std::vector<std::shared_ptr<Appointment>>&) const;
	void LoadData(std::vector<User>&, std::vector<User>&, std::vector<std::shared_ptr<Appointment>>&) const;
};