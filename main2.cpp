#include <bits/stdc++.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
using namespace std;

#ifdef _WIN32
extern "C" FILE *_popen(const char *_Command, const char *_Mode);
extern "C" int _pclose(FILE *_File);
#endif

// Function to write user input to JSON
void writeJSON(const string &filename, const map<string, string> &data)
{
    ofstream file(filename);
    file << "{\n";
    int i = 0;
    for (auto &p : data)
    {
        file << "  \"" << p.first << "\": ";
        // Add quotes for strings, no quotes for numbers
        bool isNumber = !p.second.empty() && all_of(p.second.begin(), p.second.end(),
                                                    [](unsigned char c)
                                                    { return isdigit(c) || c == '.'; });
        if (isNumber)
            file << p.second;
        else
            file << "\"" << p.second << "\"";
        if (++i < data.size())
            file << ",";
        file << "\n";
    }
    file << "}\n";
}

// Function to call Python and capture output
string runPythonAndGetOutput(const string &cmd)
{
    string result;
    char buffer[256];

#ifdef _WIN32
    FILE *pipe = _popen(cmd.c_str(), "r");
#else
    FILE *pipe = popen(cmd.c_str(), "r");
#endif

    if (!pipe)
        return "ERROR";

    while (fgets(buffer, sizeof(buffer), pipe))
        result += buffer;

#ifdef _WIN32
    _pclose(pipe);
#else
    pclose(pipe);
#endif

    return result;
}

// Struct for dataset entries
struct Listing
{
    string city, locality;
    double price;
};

int main()
{
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cout << "<---------- House Price Estimator ---------->\n\n";

    // Step 1: Take user input
    map<string, string> features;
    features["City"] = "Bengaluru";
    features["Locality"] = "HSR Layout";
    features["PropertyType"] = "Apartment";
    features["BHK"] = "3";
    features["Bathrooms"] = "2";
    features["Balconies"] = "1";
    features["Furnishing"] = "Unfurnished";
    features["SuperBuiltUpArea_sqft"] = "1768";
    features["BuiltUpArea_sqft"] = "1425";
    features["CarpetArea_sqft"] = "1211";
    features["Floor"] = "8";
    features["TotalFloors"] = "31";
    features["Parking"] = "Covered";
    features["BuildingType"] = "High Rise";
    features["YearBuilt"] = "2014";
    features["AgeYears"] = "11";
    features["Facing"] = "North";
    features["AmenitiesCount"] = "8";
    features["IsRERARegistered"] = "1";
    features["Latitude"] = "12.948053";
    features["Longitude"] = "77.785061";

    // Step 2: Write to temp JSON
    string jsonFile = "temp_features.json";
    writeJSON(jsonFile, features);

    // Step 3: Call Python script
    string output = runPythonAndGetOutput("python predict.py " + jsonFile);

    // Step 4: Parse predicted price
    double predicted_price = 0;
    size_t pos = output.find("predicted_price");
    if (pos != string::npos)
    {
        size_t colon = output.find(":", pos);
        size_t end = output.find("}", colon);
        string num = output.substr(colon + 1, end - colon - 1);
        predicted_price = stod(num);
    }
    cout << fixed << setprecision(2);
    cout << "Predicted Price: " << predicted_price << " Rupees\n\n";

    // Step 5: Load dataset.csv
    vector<Listing> listings;
    ifstream fin("dataset.csv");
    string line;
    getline(fin, line); // skip header
    while (getline(fin, line))
    {
        stringstream ss(line);
        string city, price_str, locality;
        getline(ss, city, ',');
        getline(ss, price_str, ',');
        getline(ss, locality, ',');
        if (!city.empty() && !price_str.empty())
        {
            listings.push_back({city, locality, stod(price_str)});
        }
    }
    fin.close();

    // Step 6: Search nearby listings
    double tolerance = 0.1; // ±10%
    double lower = predicted_price * (1 - tolerance);
    double upper = predicted_price * (1 + tolerance);

    // Vector linear search
    auto start = chrono::high_resolution_clock::now();
    int comparisons = 0;
    vector<Listing> nearby;
    for (auto &l : listings)
    {
        comparisons++;
        if (l.price >= lower && l.price <= upper)
            nearby.push_back(l);
    }
    auto end = chrono::high_resolution_clock::now();
    double t1 = chrono::duration<double, milli>(end - start).count();

    cout << "===== Nearby Listings (within approx(10%)) =====\n";
    for (auto &l : nearby)
        cout << "City: " << l.city << ", Locality: " << l.locality << ", Price: " << l.price << " Rupees\n";
    cout << "\nVector linear search comparisons: " << comparisons
         << ", time: " << t1 << " ms\n";
}
