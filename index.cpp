#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <vector>


using namespace std;

// === ANSI COLOR CODES FOR CONSOLE BEAUTIFICATION ===
#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define CYAN "\033[36m"
#define BOLD "\033[1m"

// Utility function to clear the input buffer
void clearBuffer() {
  cin.clear();
  cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

// ==========================================================
// 1. BASE CLASS: Document (Abstract Class)
// ==========================================================
class Document {
private:
  string documentId;
  string title;
  string publisher;
  int publishYear;

public:
  // Constructor
  Document() : documentId(""), title(""), publisher(""), publishYear(0) {}
  Document(string id, string t, string pub, int year)
      : documentId(id), title(t), publisher(pub), publishYear(year) {}
  virtual ~Document() {}

  // Full Getter / Setter
  string getDocumentId() const { return documentId; }
  void setDocumentId(string id) { documentId = id; }

  string getTitle() const { return title; }
  void setTitle(string t) { title = t; }

  string getPublisher() const { return publisher; }
  void setPublisher(string pub) { publisher = pub; }

  int getPublishYear() const { return publishYear; }
  void setPublishYear(int year) { publishYear = year; }

  // Virtual method for Polymorphism
  virtual void Input(istream &is) {
    cout << "Enter document ID: ";
    is >> documentId;
    clearBuffer();
    cout << "Enter title: ";
    getline(is, title);
    cout << "Enter publisher: ";
    getline(is, publisher);
    cout << "Enter publish year: ";
    is >> publishYear;
  }

  virtual void DisplayInfo(ostream &os) const {
    os << left << setw(10) << documentId << setw(25) << title << setw(20)
       << publisher << setw(10) << publishYear;
  }

  // Pure virtual method: Forces child classes to define price calculation
  virtual double CalculatePrice() const = 0;

  // Overloading input/output operators combined with Polymorphism
  friend istream &operator>>(istream &is, Document &doc) {
    doc.Input(is);
    return is;
  }
  friend ostream &operator<<(ostream &os, const Document &doc) {
    doc.DisplayInfo(os);
    return os;
  }
};

// ==========================================================
// 2. INHERITED CLASSES: Book, Magazine, Newspaper
// ==========================================================

class Book : public Document {
private:
  int pageCount;
  string author;

public:
  Book() : Document(), pageCount(0), author("") {}

  // Override Input method
  void Input(istream &is) override {
    Document::Input(is);
    cout << "Enter page count: ";
    is >> pageCount;
    clearBuffer();
    cout << "Enter author: ";
    getline(is, author);
  }

  // Override Display method (Use iomanip for column alignment)
  void DisplayInfo(ostream &os) const override {
    Document::DisplayInfo(os);
    os << left << setw(15) << "Book" << setw(20) << author << setw(15)
       << (to_string(pageCount) + " pages") << YELLOW << CalculatePrice() << RESET
       << endl;
  }

  // Specific price formula: Book = Page count * 500 VND
  double CalculatePrice() const override { return pageCount * 500.0; }
};

class Magazine : public Document {
private:
  int issueNumber;
  int releaseMonth;

public:
  Magazine() : Document(), issueNumber(0), releaseMonth(0) {}

  void Input(istream &is) override {
    Document::Input(is);
    cout << "Enter issue number: ";
    is >> issueNumber;
    cout << "Enter release month (1-12): ";
    is >> releaseMonth;
  }

  void DisplayInfo(ostream &os) const override {
    Document::DisplayInfo(os);
    string extraInfo =
        "Issue: " + to_string(issueNumber) + ", M" + to_string(releaseMonth);
    os << left << setw(15) << "Magazine" << setw(20) << "N/A" << setw(15)
       << extraInfo << YELLOW << CalculatePrice() << RESET << endl;
  }

  // Specific price formula: Magazine = Fixed price 25000 VND
  double CalculatePrice() const override { return 25000.0; }
};

class Newspaper : public Document {
private:
  int releaseDate;

public:
  Newspaper() : Document(), releaseDate(0) {}

  void Input(istream &is) override {
    Document::Input(is);
    cout << "Enter release date (1-31): ";
    is >> releaseDate;
  }

  void DisplayInfo(ostream &os) const override {
    Document::DisplayInfo(os);
    string extraInfo = "Date: " + to_string(releaseDate);
    os << left << setw(15) << "Newspaper" << setw(20) << "N/A" << setw(15)
       << extraInfo << YELLOW << CalculatePrice() << RESET << endl;
  }

  // Specific price formula: Newspaper = Fixed price 5000 VND
  double CalculatePrice() const override { return 5000.0; }
};

// ==========================================================
// 3. MAIN PROGRAM
// ==========================================================
void PrintTableHeader() {
  cout << string(125, '-') << endl;
  cout << BOLD << CYAN << left << setw(10) << "Doc ID" << setw(25)
       << "Title" << setw(20) << "Publisher" << setw(10) << "Year"
       << setw(15) << "Category" << setw(20) << "Author" << setw(15)
       << "Details"
       << "Price" << RESET << endl;
  cout << string(125, '-') << endl;
}

int main() {
  // Use Vector of base class pointers to demonstrate Polymorphism
  vector<Document *> documentList;
  int choice;

  do {
    cout << "\n"
         << BOLD << GREEN << "=== LIBRARY MANAGEMENT SYSTEM ===" << RESET
         << "\n";
    cout << "1. Add Book\n";
    cout << "2. Add Magazine\n";
    cout << "3. Add Newspaper\n";
    cout << "4. Display List and Calculate Price\n";
    cout << "0. Exit\n";
    cout << "Enter your choice: ";
    cin >> choice;

    Document *newDoc = nullptr;

    switch (choice) {
    case 1:
      cout << CYAN << "\n--- Enter Book details ---" << RESET << endl;
      newDoc = new Book();
      break;
    case 2:
      cout << CYAN << "\n--- Enter Magazine details ---" << RESET << endl;
      newDoc = new Magazine();
      break;
    case 3:
      cout << CYAN << "\n--- Enter Newspaper details ---" << RESET << endl;
      newDoc = new Newspaper();
      break;
    case 4:
      if (documentList.empty()) {
        cout << RED << "List is empty!" << RESET << endl;
      } else {
        cout << "\n" << BOLD << GREEN << "DOCUMENT LIST" << RESET << "\n";
        PrintTableHeader();
        double totalPrice = 0;
        for (const auto &doc : documentList) {
          // Call output operator (<<), automatically triggering Polymorphism
          cout << *doc;
          totalPrice += doc->CalculatePrice();
        }
        cout << string(125, '-') << endl;
        cout << BOLD << RED << "TOTAL PRICE OF ALL DOCUMENTS: " << fixed
             << setprecision(0) << totalPrice << " VND" << RESET << endl;
      }
      break;
    case 0:
      cout << "Exiting program...\n";
      break;
    default:
      cout << RED << "Invalid choice. Please try again!\n" << RESET;
      clearBuffer();
    }

    // If successfully created, call input operator (>>) triggering Polymorphism
    if (newDoc != nullptr) {
      cin >> *newDoc;
      documentList.push_back(newDoc);
      cout << GREEN << "Document added successfully!\n" << RESET;
    }

  } while (choice != 0);

  // Free memory (Mandatory rule when using pointers)
  for (auto doc : documentList) {
    delete doc;
  }
  documentList.clear();

  return 0;
}