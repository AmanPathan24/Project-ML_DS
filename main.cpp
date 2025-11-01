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

// ------------------------- DSA STRUCTURES -------------------------

struct Listing
{
    string city, locality;
    double price;
};

// Linked list node
struct Node
{
    Listing data;
    Node *next;
    Node(Listing l) : data(l), next(nullptr) {}
};

// BST Node
struct BSTNode
{
    Listing data;
    BSTNode *left, *right;
    BSTNode(Listing l) : data(l), left(nullptr), right(nullptr) {}
};

// Insert into BST
BSTNode *insertBST(BSTNode *root, Listing l)
{
    if (!root)
        return new BSTNode(l);
    if (l.price < root->data.price)
        root->left = insertBST(root->left, l);
    else
        root->right = insertBST(root->right, l);
    return root;
}

// Search BST for nearby prices
void searchBST(BSTNode *root, double low, double high, vector<Listing> &res, int &comparisons)
{
    if (!root)
        return;
    comparisons++;
    if (root->data.price >= low && root->data.price <= high)
        res.push_back(root->data);
    if (root->data.price > low)
        searchBST(root->left, low, high, res, comparisons);
    if (root->data.price < high)
        searchBST(root->right, low, high, res, comparisons);
}

// ------------------------- MAIN -------------------------

int main()
{
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // Small helpers for nicer output
    auto repeat = [](char c, int n)
    {
        return string(max(0, n), c);
    };

    auto formatMoney = [](double value) -> string
    {
        // format with two decimals and commas in integer part, prefix with ₹
        ostringstream oss;
        oss << fixed << setprecision(2) << value;
        string s = oss.str(); // e.g. "1234567.89"
        size_t dot = s.find('.');
        string intpart = (dot == string::npos) ? s : s.substr(0, dot);
        string fracpart = (dot == string::npos) ? "" : s.substr(dot); // includes '.'
        string out;
        int cnt = 0;
        for (int i = (int)intpart.size() - 1; i >= 0; --i)
        {
            out.push_back(intpart[i]);
            if (++cnt == 3 && i > 0)
            {
                out.push_back(',');
                cnt = 0;
            }
        }
        reverse(out.begin(), out.end());
        return out + fracpart;
    };

    auto pctDiff = [](double base, double other) -> double
    {
        if (base == 0)
            return 0.0;
        return (other - base) / base * 100.0;
    };

    cout << "\n"
         << repeat('=', 70) << "\n";
    cout << setw(36) << right << "<------------ House Price Estimator ------------>" << "\n";
    cout << repeat('=', 70) << "\n\n";

    // Step 1: Input Features (can be replaced with user input)
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

    // Show features summary
    cout << "Input features:\n";
    for (auto &p : features)
    {
        cout << "  " << left << setw(25) << p.first << ": " << p.second << "\n";
    }
    cout << "\n";

    // Step 2: Write to temp JSON
    string jsonFile = "temp_features.json";
    writeJSON(jsonFile, features);

    // Step 3: Call Python prediction
    cout << "Running model to predict price...\n";
    string output = runPythonAndGetOutput("python predict.py " + jsonFile);

    // Step 4: Extract predicted price
    double predicted_price = 0;
    size_t pos = output.find("predicted_price");
    if (pos != string::npos)
    {
        size_t colon = output.find(":", pos);
        size_t end = output.find_first_of("}\n", colon);
        if (colon != string::npos && end != string::npos && end > colon)
        {
            string num = output.substr(colon + 1, end - colon - 1);
            // trim spaces
            num.erase(num.begin(), find_if(num.begin(), num.end(), [](int ch)
                                           { return !isspace(ch); }));
            num.erase(find_if(num.rbegin(), num.rend(), [](int ch)
                              { return !isspace(ch); })
                          .base(),
                      num.end());
            try
            {
                predicted_price = stod(num);
            }
            catch (...)
            {
                predicted_price = 0;
            }
        }
    }

    cout << "\n"
         << repeat('-', 70) << "\n";
    cout << "Predicted Price:  " << setw(30) << left << formatMoney(predicted_price) << "\n";
    cout << repeat('-', 70) << "\n\n";

    // Step 5: Load dataset.csv
    vector<Listing> listings;
    ifstream fin("dataset.csv");
    if (!fin)
    {
        cerr << "Error: dataset.csv not found. Exiting.\n";
        return 1;
    }
    string line;
    getline(fin, line);
    while (getline(fin, line))
    {
        stringstream ss(line);
        string city, price_str, locality;
        getline(ss, city, ',');
        getline(ss, price_str, ',');
        getline(ss, locality, ',');
        if (!city.empty() && !price_str.empty())
        {
            try
            {
                listings.push_back({city, locality, stod(price_str)});
            }
            catch (...)
            {
            }
        }
    }
    fin.close();

    double tol = 0.10; // 10%
    double low = predicted_price * (1 - tol);
    double high = predicted_price * (1 + tol);

    // ------------------------- VECTOR SEARCH -------------------------
    auto start = chrono::high_resolution_clock::now();
    int comp1 = 0;
    vector<Listing> res1;
    for (auto &l : listings)
    {
        comp1++;
        if (l.price >= low && l.price <= high)
            res1.push_back(l);
    }
    auto end = chrono::high_resolution_clock::now();
    double t_vec = chrono::duration<double, milli>(end - start).count();

    // Sort results by closeness to predicted price and keep top 10
    sort(res1.begin(), res1.end(), [&](const Listing &a, const Listing &b)
         { return fabs(a.price - predicted_price) < fabs(b.price - predicted_price); });
    if (res1.size() > 10)
        res1.resize(10);

    // ------------------------- LINKED LIST SEARCH -------------------------
    Node *head = nullptr, *tail = nullptr;
    for (auto &l : listings)
    {
        Node *n = new Node(l);
        if (!head)
            head = n;
        else
            tail->next = n;
        tail = n;
    }

    start = chrono::high_resolution_clock::now();
    int comp2 = 0;
    vector<Listing> res2;
    for (Node *cur = head; cur; cur = cur->next)
    {
        comp2++;
        if (cur->data.price >= low && cur->data.price <= high)
            res2.push_back(cur->data);
    }
    end = chrono::high_resolution_clock::now();
    double t_ll = chrono::duration<double, milli>(end - start).count();

    // free linked list nodes
    while (head)
    {
        Node *tmp = head;
        head = head->next;
        delete tmp;
    }

    // ------------------------- BST SEARCH -------------------------
    BSTNode *root = nullptr;
    for (auto &l : listings)
        root = insertBST(root, l);

    start = chrono::high_resolution_clock::now();
    int comp3 = 0;
    vector<Listing> res3;
    searchBST(root, low, high, res3, comp3);
    end = chrono::high_resolution_clock::now();
    double t_bst = chrono::duration<double, milli>(end - start).count();

    // ------------------------- HASH TABLE SEARCH -------------------------
    unordered_map<int, vector<Listing>> hashmap;
    for (auto &l : listings)
        hashmap[(int)(l.price / 100000)].push_back(l);

    start = chrono::high_resolution_clock::now();
    int comp4 = 0;
    vector<Listing> res4;
    int keyLow = (int)(low / 100000), keyHigh = (int)(high / 100000);
    for (int k = keyLow; k <= keyHigh; ++k)
    {
        if (hashmap.find(k) != hashmap.end())
        {
            for (auto &l : hashmap[k])
            {
                comp4++;
                if (l.price >= low && l.price <= high)
                    res4.push_back(l);
            }
        }
    }
    end = chrono::high_resolution_clock::now();
    double t_hash = chrono::duration<double, milli>(end - start).count();

    // ------------------------- OUTPUT -------------------------
    cout << "\n"
         << repeat('=', 70) << "\n";
    cout << "Nearby Listings (within 10% range) - showing top " << res1.size() << " closest matches\n";
    cout << repeat('-', 70) << "\n";
    cout << left << setw(5) << "No."
         << setw(20) << "City"
         << setw(25) << "Locality"
         << setw(18) << "Price"
         << setw(12) << "Diff(%)" << "\n";
    cout << repeat('-', 70) << "\n";

    for (size_t i = 0; i < res1.size(); ++i)
    {
        const auto &l = res1[i];
        double diff = pctDiff(predicted_price, l.price);
        cout << left << setw(5) << (i + 1)
             << setw(20) << (l.city.size() > 18 ? l.city.substr(0, 17) + "…" : l.city)
             << setw(25) << (l.locality.size() > 23 ? l.locality.substr(0, 22) + "…" : l.locality)
             << setw(18) << formatMoney(l.price)
             << setw(11) << fixed << setprecision(2) << diff << fixed << setprecision(2) << "%\n";
    }

    cout << repeat('=', 70) << "\n\n";

    cout << "Performance Comparison (measured on this run):\n\n";
    cout << left << setw(30) << "Data Structure"
         << setw(18) << "Comparisons"
         << setw(16) << "Time (ms)"
         << setw(14) << "Results\n";
    cout << repeat('-', 78) << "\n";
    cout << left << setw(30) << "Vector (Linear Search)"
         << setw(18) << comp1
         << setw(16) << fixed << setprecision(3) << t_vec
         << setw(14) << res1.size() << "\n";
    cout << left << setw(30) << "Linked List"
         << setw(18) << comp2
         << setw(16) << fixed << setprecision(3) << t_ll
         << setw(14) << res2.size() << "\n";
    cout << left << setw(30) << "Binary Search Tree"
         << setw(18) << comp3
         << setw(16) << fixed << setprecision(3) << t_bst
         << setw(14) << res3.size() << "\n";
    cout << left << setw(30) << "Hash Table (bucketed)"
         << setw(18) << comp4
         << setw(16) << fixed << setprecision(3) << t_hash
         << setw(14) << res4.size() << "\n";
    cout << repeat('-', 78) << "\n\n";

    cout << "Notes:\n";
    cout << " - Results sorted by closeness to predicted price.\n";
    cout << " - Times are wall-clock milliseconds for the search operation only.\n\n";

    cout << repeat('=', 70) << "\n";
    cout << setw(40) << right << "End of Report\n";
    cout << repeat('=', 70) << "\n\n";

    return 0;
}
