// Nạp các thư viện chuẩn cần thiết cho chương trình
#include <algorithm> // Thư viện chứa std::transform
#include <cctype>    // Thư viện chứa ::tolower
#include <ctime>     // Cung cấp các hàm lấy và xử lý thời gian hệ thống
#include <fstream>   // Thư viện dùng để đọc/ghi file dữ liệu
#include <iomanip> // Cung cấp các thao tác định dạng căn trích như setw (cột) và left (căn trái)
#include <iostream> // Cung cấp các luồng nhập/xuất chuẩn như cin, cout
#include <limits> // Cung cấp giới hạn kiểu số, dùng hữu ích khi kết hợp thao tác xóa bộ nhớ đệm
#include <sstream> // Thư viện dùng để xử lý cắt chuỗi (parsing) dữ liệu từ file
#include <string>
#include <vector>

#ifndef NOMINMAX
#define NOMINMAX // Định nghĩa rào chắn để tránh xung đột hàm min/max khi gọi
                 // windows.h
#endif
#include <windows.h> // Khai báo file thư viện hệ thống Windows, phục vụ cài đặt định dạng hiển thị UTF-8

using namespace std;

// === ĐỊNH NGHĨA MÃ MÀU ANSI ĐỂ LÀM ĐẸP CONSOLE (2.0đ) ===
// Quy định hằng chuỗi mã hóa để kích hoạt hệ thống đổi màu văn bản hiển thị
#define RESET "\033[0m" // Dùng để reset màu sắc văn bản trở về mặc định chung
#define RED "\033[31m"  // Đặt màu đỏ dùng cho các cảnh báo hoặc lỗi nhập
#define GREEN                                                                  \
  "\033[32m" // Đặt màu xanh lá dành riêng cho các thành công/tiêu đề
#define YELLOW "\033[33m" // Đặt màu vàng tạo điểm nhấn (phần hiển thị giá tiền)
#define CYAN                                                                   \
  "\033[36m"           // Đặt màu xanh ngọc cho phần nổi bật của tiêu đề bản ghi
#define BOLD "\033[1m" // Mã đặc biệt giúp bôi đậm văn bản cần chú ý

// Hàm tiện ích để làm sạch bộ nhớ đệm, dùng để chống trôi lệnh khi xen kẽ giữa
// getline() và cin >>
void clearBuffer() {
  cin.clear(); // Hủy cờ lỗi trên luồng nhập khi người dùng lỡ nhập sai kiểu dữ
               // liệu chữ vào số
  cin.ignore(
      numeric_limits<streamsize>::max(),
      '\n'); // Loại bỏ tất cả ký tự tồn đọng bao gồm cả ký tự Enter ('\n')
}

// Hàm hỗ trợ làm sạch ký tự '|' để tránh lỗi file database
void sanitizeString(string &s) { replace(s.begin(), s.end(), '|', ' '); }

// ==========================================================
// 0. LỚP NGƯỜI DÙNG: User vả Xác Thực (Authentication)
// ==========================================================

// Giải thích: Hệ thống phân quyền, sử dụng để tạo đối tượng lưu trữ Account
// đăng nhập.
class User {
private:
  string username;
  string password;
  string role;

public:
  User(string uname, string pwd, string r)
      : username(uname), password(pwd), role(r) {}
  virtual ~User() {}

  string getUsername() const { return username; }
  string getPassword() const { return password; }
  string getRole() const { return role; }
  void setPassword(string pwd) { password = pwd; }
};

// Lớp Librarian (Thủ thư): Đại diện cho nhân viên thư viện, có các quyền quản
// lý đầy đủ (Thêm/Xóa)
class Librarian : public User {
public:
  Librarian(string uname, string pwd) : User(uname, pwd, "Librarian") {}
};

// Lớp Reader (Độc giả): Đại diện cho người dùng bình thường, chỉ có quyền
// Mượn/Trả/Tìm kiếm
class Reader : public User {
public:
  Reader(string uname, string pwd) : User(uname, pwd, "Reader") {}
};

// Lớp AuthManager (Quản lý Xác thực): Đảm nhận vai trò kiểm tra đăng nhập và
// quản lý danh sách tài khoản
class AuthManager {
public:
  // Hàm taoFileChuan: Tạo ra file db account mặc định nếu hệ thống chưa có sẵn
  // file users.txt
  static void taoFileChuan() {
    ifstream ifs("users.txt");
    if (!ifs.is_open()) {
      ofstream ofs("users.txt");
      ofs << "admin|admin123|Librarian\n";
      ofs << "john|john123|Reader\n";
      ofs.close();
      cout << GREEN
           << "(!) Đã khởi tạo file users.txt với tài khoản mặc định "
              "(admin/john)."
           << RESET << "\n";
    }
  }

  // Hàm login: Tạo vòng lặp yêu cầu người dùng nhập tên và mật khẩu. Trả về
  // đúng đối tượng User theo quyền nếu khớp file.
  static void dangKy() {
    string newUser, newPwd;
    cout << "\n"
         << BOLD << CYAN << "=== ĐĂNG KÝ TÀI KHOẢN MỚI ===" << RESET << "\n";
    cout << "Tên tài khoản: ";
    cin >> newUser;

    ifstream ifs("users.txt");
    string line, uname, pwd, role;
    bool exists = false;
    while (getline(ifs, line)) {
      stringstream ss(line);
      getline(ss, uname, '|');
      if (uname == newUser) {
        exists = true;
        break;
      }
    }
    ifs.close();

    if (exists) {
      cout << RED << "Tài khoản đã tồn tại! Vui lòng chọn tên khác." << RESET
           << "\n";
      return;
    }

    cout << "Mật khẩu: ";
    cin >> newPwd;

    ofstream ofs("users.txt", ios::app);
    if (!ofs) {
      cout << RED << "Lỗi mở file users.txt" << RESET << "\n";
      return;
    }
    ofs << newUser << "|" << newPwd << "|Reader\n";
    ofs.close();
    cout << GREEN << "Đăng ký thành công! Bạn có thể đăng nhập ngay bây giờ."
         << RESET << "\n";
  }

  static void doiMatKhau(User *currentUser) {
    string currentPwd, newPwd;
    cout << "\n" << BOLD << CYAN << "=== ĐỔI MẬT KHẨU ===" << RESET << "\n";
    cout << "Nhập mật khẩu hiện tại: ";
    cin >> currentPwd;

    if (currentPwd != currentUser->getPassword()) {
      cout << RED << "Mật khẩu hiện tại không đúng!" << RESET << "\n";
      return;
    }

    cout << "Nhập mật khẩu mới: ";
    cin >> newPwd;

    ifstream ifs("users.txt");
    vector<string> lines;
    string line;
    while (getline(ifs, line)) {
      stringstream ss(line);
      string uname, pwd, role;
      getline(ss, uname, '|');
      getline(ss, pwd, '|');
      getline(ss, role, '|');

      if (uname == currentUser->getUsername()) {
        lines.push_back(uname + "|" + newPwd + "|" + role);
      } else {
        lines.push_back(line);
      }
    }
    ifs.close();

    ofstream ofs("users.txt");
    for (const string &l : lines) {
      ofs << l << "\n";
    }
    ofs.close();

    currentUser->setPassword(newPwd);
    cout << GREEN << "Đổi mật khẩu thành công!" << RESET << "\n";
  }

  static User *login() {
    taoFileChuan();
    int choice;
    while (true) {
      cout << "\n"
           // Dùng công cụ stringstream cắt xẻ chuỗi trên một dòng dựa trên dấu
           // vạch đứng '|'
           << BOLD << CYAN << "=== TRANG CHỦ THƯ VIỆN ===" << RESET << "\n";
      cout << "1. Đăng nhập\n";
      cout << "2. Đăng ký (Dành cho Reader)\n";
      cout << "0. Thoát\n";
      cout << "Lựa chọn: ";
      if (!(cin >> choice)) {
        cout << RED << "Lựa chọn không hợp lệ!" << RESET << "\n";
        clearBuffer();
        continue;
      }

      if (choice == 0)
        return nullptr;
      if (choice == 2) {
        dangKy();
        continue;
      }
      if (choice == 1) {
        string reqUser, reqPwd;
        cout << "Tài khoản: ";
        cin >> reqUser;
        cout << "Mật khẩu: ";
        cin >> reqPwd;

        ifstream ifs("users.txt");
        string line;
        bool found = false;
        while (getline(ifs, line)) {
          stringstream ss(line);
          string uname, pwd, role;
          getline(ss, uname, '|');
          getline(ss, pwd, '|');
          getline(ss, role, '|');

          if (uname == reqUser && pwd == reqPwd) {
            cout << GREEN << "\nĐăng nhập thành công! Vai trò: " << role
                 << RESET << "\n";
            found = true;
            if (role == "Librarian")
              return new Librarian(uname, pwd);
            else
              return new Reader(uname, pwd);
          }
        }
        if (!found) {
          cout << RED << "Tài khoản hoặc mật khẩu không chính xác. Thử lại!"
               << RESET << "\n";
        }
      } else {
        cout << RED << "Lựa chọn không hợp lệ!" << RESET << "\n";
      }
    }
  }
};

// ==========================================================
// 1. LỚP CƠ SỞ: TaiLieu (Abstract Class - Được coi là Lớp Trừu Tượng)
// ==========================================================
// Giải thích: Lớp trừu tượng (Lớp Cha) chứa các thông tin chung nhất. Lớp này
// không thể khởi tạo trực tiếp.
class TaiLieu {
private:
  // Khai báo các thuộc tính chung chỉ cho quyền nội bộ theo đóng gói
  // (Encapsulation)
  string maTaiLieu;
  string tenTaiLieu;
  string nhaXuatBan;
  int namXuatBan;

protected:
  bool isBorrowed;
  string borrowerName;

public:
  // Hàm Constructor (Khởi tạo) không tham số thiết lập giá trị chuẩn an toàn
  // lúc rỗng
  TaiLieu()
      : maTaiLieu(""), tenTaiLieu(""), nhaXuatBan(""), namXuatBan(0),
        isBorrowed(false), borrowerName("") {}

  // Hàm Constructor có nhận các tham số dùng để khởi gán đối tượng lập tức
  TaiLieu(string ma, string ten, string nxb, int nam)
      : maTaiLieu(ma), tenTaiLieu(ten), nhaXuatBan(nxb), namXuatBan(nam),
        isBorrowed(false), borrowerName("") {}

  // Hàm Destructor ảo đảm bảo máy tính xóa đúng bộ nhớ rác khi đối tượng dẫn
  // xuất kết thúc
  virtual ~TaiLieu() {}

  // Tập hợp Getter / Setter theo giao diện điều khiển (interface) an toàn
  // lấy/gán dữ liệu (2.0đ)
  string getMaTaiLieu() const { return maTaiLieu; }
  // Bộ Getter/Setter: Dùng để an toàn lấy (get) hoặc ghi đè (set) dữ liệu riêng
  // tư từ bên ngoài lớp
  void setMaTaiLieu(string ma) { maTaiLieu = ma; }

  string getTenTaiLieu() const { return tenTaiLieu; }
  void setTenTaiLieu(string ten) { tenTaiLieu = ten; }

  string getNhaXuatBan() const { return nhaXuatBan; }
  void setNhaXuatBan(string nxb) { nhaXuatBan = nxb; }

  int getNamXuatBan() const { return namXuatBan; }
  void setNamXuatBan(int nam) { namXuatBan = nam; }

  bool getIsBorrowed() const { return isBorrowed; }
  void setIsBorrowed(bool status) { isBorrowed = status; }

  string getBorrowerName() const { return borrowerName; }
  void setBorrowerName(string name) { borrowerName = name; }

  // Phương thức Đa hình (ảo hóa - virtual) để xử lý bước nhập từng dữ liệu gốc
  // riêng Có thể được ghi đè bởi các lớp Sách/Báo nhờ cơ chế liên kết trễ (Late
  // binding) - (3.0đ)
  // Giải thích: Hàm nhập thuần ảo (Đa hình).
  virtual void Nhap(istream &is, const vector<TaiLieu *> &danhSach) = 0;

  // Khai báo hàm ảo hiển thị chuỗi nội dung với việc đẩy cột thẳng hàng
  virtual void HienThiThongTin(ostream &os) const = 0;

  // Phương thức thuần ảo ép lớp nhận kế thừa bắt buộc phải viết lại
  // Hàm này chặn người dùng khởi tạo trực tiếp instance ảo Tài Liệu không xác
  // định
  // Giải thích: Hàm thuần ảo (Đa hình). Bắt buộc các mục Sách, Báo, Tạp chí
  // phải tự viết lại công thức tính tiền riêng.
  virtual double TinhTien() const = 0;

  // Phương thức ảo để lưu dữ liệu vào stream (phục vụ lưu file)
  virtual void Luu(ostream &os) const = 0;

  // Phương thức ảo trả về loại tài liệu (1=Sách, 2=Tạp Chí, 3=Báo)
  virtual int getLoaiTaiLieu() const = 0;

  // Phương thức ảo để sửa thông tin
  virtual void SuaThongTin() = 0;

  // Phương thức ảo lấy chuỗi chi tiết cho tìm kiếm sâu
  virtual string getChiTietHienThi() const { return ""; }

  // Phương thức hỗ trợ sửa thông tin chung
  void SuaThongTinChung() {
    string tmp;
    cout << "Tên tài liệu (" << tenTaiLieu
         << "). Nhập mới hoặc Enter để bỏ qua: ";
    getline(cin, tmp);
    if (!tmp.empty()) {
      tenTaiLieu = tmp;
      sanitizeString(tenTaiLieu);
    }

    cout << "Nhà Xuất Bản (" << nhaXuatBan
         << "). Nhập mới hoặc Enter để bỏ qua: ";
    getline(cin, tmp);
    if (!tmp.empty()) {
      nhaXuatBan = tmp;
      sanitizeString(nhaXuatBan);
    }

    cout << "Năm Xuất Bản (" << namXuatBan
         << "). Nhập mới hoặc Enter để bỏ qua: ";
    getline(cin, tmp);
    if (!tmp.empty()) {
      try {
        namXuatBan = stoi(tmp);
      } catch (...) {
      }
    }
  }
};

// Cung cấp định nghĩa cho các hàm thuần ảo để các lớp con có thể tái sử dụng
// logic nhập gốc
// Hàm Nhap (của Lớp cơ sở): Xử lý nhập 4 trường thông tin dùng chung cốt lõi và
// kiểm tra trùng lặp mã.
inline void TaiLieu::Nhap(istream &is, const vector<TaiLieu *> &danhSach) {
  // Giải thuật kiểm tra mã tài liệu duy nhất
  while (true) {
    cout << "Nhập mã tài liệu: ";
    is >> maTaiLieu;
    sanitizeString(maTaiLieu);
    bool biTrùng = false;
    for (const auto &tl : danhSach) {
      if (tl != nullptr && tl->getMaTaiLieu() == maTaiLieu) {
        biTrùng = true;
        break;
      }
    }
    if (biTrùng) {
      cout << RED << "Lỗi: Mã tài liệu '" << maTaiLieu
           << "' đã tồn tại! Vui lòng nhập mã khác." << RESET << endl;
    } else {
      break;
    }
  }

  clearBuffer(); // Phải dọn bộ nhớ ngay sau đó để chuẩn bị lấy getline có đoạn
                 // trống

  cout << "Nhập tên tài liệu: ";
  getline(is, tenTaiLieu); // Đọc trọn cả dòng kể cả khoảng trắng xen kẽ
  sanitizeString(tenTaiLieu);

  cout << "Nhập nhà xuất bản: ";
  getline(is, nhaXuatBan);
  sanitizeString(nhaXuatBan);

  // Thuật toán kiểm soát lỗi nhập số cho Năm Xuất Bản
  do {
    if (is.fail()) {
      is.clear();
      is.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    cout << "Nhập năm xuất bản: ";
    is >> namXuatBan;
    if (is.fail() || namXuatBan < 0) {
      cout << RED << "Lỗi: Năm xuất bản không hợp lệ. Vui lòng nhập lại!"
           << RESET << endl;
    }
  } while (is.fail() || namXuatBan < 0);
}

// Cung cấp định nghĩa hiển thị gốc của đối tượng trừu tượng
// Hàm HienThiThongTin (của Lớp cơ sở): Dàn khung và đẩy ra màn hình các thông
// tin chung của một khối tài liệu.
inline void TaiLieu::HienThiThongTin(ostream &os) const {
  // Ép in về phía trái của cột, với cỡ chiều rộng setw cố định cho từng loại
  // thông tin
  os << left << setw(10) << maTaiLieu << setw(25) << tenTaiLieu << setw(20)
     << nhaXuatBan << setw(10) << namXuatBan;

  // Định dạng lại cột "Tình Trạng" (25 visual columns) bằng thuật toán bù byte
  if (isBorrowed) {
    os << setw(15 + sizeof("Mượn bởi: ") - 1) << ("Mượn bởi: " + borrowerName);
  } else {
    os << setw(17 + sizeof("Sẵn sàng") - 1) << "Sẵn sàng";
  }
}

// Định nghĩa phần lưu cơ sở cho các thuộc tính chung
inline void TaiLieu::Luu(ostream &os) const {
  os << maTaiLieu << "|" << tenTaiLieu << "|" << nhaXuatBan << "|" << namXuatBan
     << "|" << isBorrowed << "|" << borrowerName;
}

// ==========================================================
// 2. LỚP KẾ THỪA: Đối tượng Sách, Tạp chí, và Báo (Phần Đa Hình)
// ==========================================================

class Sach : public TaiLieu { // Khởi xướng Kế Thừa tính năng gốc từ lớp cha lớp
                              // 'TaiLieu'
private:
  // Khai báo bổ sung đặc thù chỉ cho một cuốn Sách
  int soTrang;
  string tacGia;

public:
  // Khởi tạo thông tin nội tại cuốn Sách là trống ban đầu
  Sach() : TaiLieu(), soTrang(0), tacGia("") {}

  // Nạp toàn bộ dữ liệu từ gốc đến ngọn nếu có thể xác định sớm lúc khởi tạo
  // Sách
  Sach(string ma, string ten, string nxb, int nam, int st, string tg)
      : TaiLieu(ma, ten, nxb, nam), soTrang(st), tacGia(tg) {}

  // Lấy hoặc gán lượng trang của loại Sách
  int getSoTrang() const { return soTrang; }
  void setSoTrang(int st) { soTrang = st; }

  // Lấy hoặc gán đặc tả tác giả của đối tượng Sách
  string getTacGia() const { return tacGia; }
  void setTacGia(string tg) { tacGia = tg; }

  // Ghi đè hàm Nhập() của Lớp TaiLieu để lấy thêm Tác giả và Số trang
  // Hàm Nhap (Được Ghi đè ở lớp con): Gọi nhờ hàm Nhập chung trước, kế tiếp yêu
  // cầu người dùng gõ thêm thông tin đặc thù.
  void Nhap(istream &is, const vector<TaiLieu *> &danhSach) override {
    TaiLieu::Nhap(is, danhSach); // Ra lệnh gọi phần Nhập của lớp cơ sở giải
                                 // quyết 4 thông số nền đè trước
    // Thuật toán kiểm soát lỗi nhập số cho Số Trang
    do {
      if (is.fail()) {
        is.clear();
        is.ignore(numeric_limits<streamsize>::max(), '\n');
      }
      cout << "Nhập số trang: ";
      is >> soTrang;
      if (is.fail() || soTrang <= 0) {
        cout << RED
             << "Lỗi: Số trang phải là số nguyên dương. Vui lòng nhập lại!"
             << RESET << endl;
      }
    } while (is.fail() || soTrang <= 0);

    clearBuffer(); // Đề phòng lệnh getline phía sau bị nhảy do cặn Enter
    cout << "Nhập tác giả: ";
    getline(is, tacGia); // Lấy tên tác giả (cho phép khoảng cách)
    sanitizeString(tacGia);
  }

  // Ghi đè phương thức Hiện thị (Tùy chỉnh khoảng cột với phép bù sizeof để
  // UTF-8 hoạt động chuẩn)
  // Hàm HienThiThongTin (Được Ghi đè ở lớp con): Ráp nối các cột thông tin độc
  // quyền đằng sau phần khung thông tin chuẩn.
  void HienThiThongTin(ostream &os) const override {
    TaiLieu::HienThiThongTin(os); // In 4 trường mặc định gốc trước
    // Căn trái và đẩy các trường thêm: Phân Loại (Sách), Tác Giả, Số Trang, và
    // Tinh Tiền đi kèm định dạng màu
    os << left << setw(11 + sizeof("Sách") - 1) << "Sách" << setw(20) << tacGia
       << setw(15) << (to_string(soTrang) + " trang") << YELLOW << setw(15)
       << fixed << setprecision(0) << TinhTien() << RESET << endl;
  }

  // Cấu hình quy tắc riêng trả về phép tính Tiền dựa vào Số Trang
  double TinhTien() const override {
    return soTrang * 500.0;
  } // Đồng giá 500đ nhân với lượng trang

  // Ghi đè phương thức lưu cho Sách (Type 1)
  void Luu(ostream &os) const override {
    os << "1|";
    TaiLieu::Luu(os);
    os << "|" << soTrang << "|" << tacGia << endl;
  }

  int getLoaiTaiLieu() const override { return 1; }

  void SuaThongTin() override {
    clearBuffer();
    cout << "--- SỬA THÔNG TIN SÁCH ---\n";
    TaiLieu::SuaThongTinChung();
    string tmp;
    cout << "Tác giả (" << tacGia << "). Nhập mới hoặc Enter để bỏ qua: ";
    getline(cin, tmp);
    if (!tmp.empty()) {
      tacGia = tmp;
      sanitizeString(tacGia);
    }

    cout << "Số trang (" << soTrang << "). Nhập mới hoặc Enter để bỏ qua: ";
    getline(cin, tmp);
    if (!tmp.empty()) {
      try {
        soTrang = stoi(tmp);
      } catch (...) {
      }
    }
  }

  string getChiTietHienThi() const override { return tacGia; }

  // Toán tử quá tải nhập/xuất riêng cho lớp Sách (khớp với sơ đồ UML)
  // Quá tải toán tử nhúng (>>): Cho phép lệnh cin tác động trực tiếp lên nguyên
  // một khối Sách s
  friend istream &operator>>(istream &is, Sach &s) {
    // Lưu ý: operator>> không có truy cập vào danhSach để check duy nhất,
    // nên khuyến khích dùng Nhap(is, danhSach) trực tiếp.
    s.Nhap(is, vector<TaiLieu *>());
    return is;
  }

  // Quá tải toán tử in (<<): Giúp lệnh cout in thẳng đối tượng Sách s thay vì
  // phải gọi s.HienThiThongTin()
  friend ostream &operator<<(ostream &os, Sach &s) {
    s.HienThiThongTin(os);
    return os;
  }
};

class TapChi : public TaiLieu { // Gốc tạo đối tượng loại biên bản mới 'TapChi'
private:
  // Xác định định kỳ thông tin
  int soPhatHanh;
  int thangPhatHanh;

public:
  TapChi() : TaiLieu(), soPhatHanh(0), thangPhatHanh(0) {}
  TapChi(string ma, string ten, string nxb, int nam, int sph, int tph)
      : TaiLieu(ma, ten, nxb, nam), soPhatHanh(sph), thangPhatHanh(tph) {}

  int getSoPhatHanh() const { return soPhatHanh; }
  void setSoPhatHanh(int sph) { soPhatHanh = sph; }

  int getThangPhatHanh() const { return thangPhatHanh; }
  void setThangPhatHanh(int tph) { thangPhatHanh = tph; }

  // Kịch bản Nhập thêm đối với loại Tạp Chí yêu cầu rà soát giá trị tháng (từ
  // 1-12)
  // Hàm Nhap (Được Ghi đè ở lớp con): Gọi nhờ hàm Nhập chung trước, kế tiếp yêu
  // cầu người dùng gõ thêm thông tin đặc thù.
  void Nhap(istream &is, const vector<TaiLieu *> &danhSach) override {
    TaiLieu::Nhap(is, danhSach); // Điền thông số chuẩn gốc cho Tạp Chí
    // Thuật toán kiểm soát lỗi cho Số Phát Hành
    do {
      if (is.fail()) {
        is.clear();
        is.ignore(numeric_limits<streamsize>::max(), '\n');
      }
      cout << "Nhập số phát hành: ";
      is >> soPhatHanh;
      if (is.fail() || soPhatHanh < 0) {
        cout << RED << "Lỗi: Số phát hành không hợp lệ. Vui lòng nhập lại!"
             << RESET << endl;
      }
    } while (is.fail() || soPhatHanh < 0);

    // Khởi tạo khoá cản ngoại lệ kiểm soát các trị số nhập hợp lệ theo tháng (1
    // đến 12)
    do {
      if (is.fail()) { // Bắt lỗi gõ luồng (Vd: nhập chữ vào int) để tránh vòng
                       // lặp chết
        is.clear();
        is.ignore(numeric_limits<streamsize>::max(), '\n');
      }
      cout << "Nhập tháng phát hành (1-12): ";
      is >> thangPhatHanh;

      // Nếu lọt luồng fail hoặc trị số bị lố ngoài 1 và 12, cảnh báo bằng màu
      // RED yêu cầu thử lại
      if (is.fail() || thangPhatHanh < 1 || thangPhatHanh > 12) {
        cout << RED
             << "Lỗi: Tháng phát hành phải từ 1 đến 12. Vui lòng nhập lại!"
             << RESET << endl;
      }
    } while (is.fail() || thangPhatHanh < 1 ||
             thangPhatHanh > 12); // Bẻ vòng khi kết quả hoàn toàn đúng đắn
  }

  // Khai báo hàm đa hình cho bộ hiển thị dữ liệu bảng điều khiển
  // Hàm HienThiThongTin (Được Ghi đè ở lớp con): Ráp nối các cột thông tin độc
  // quyền đằng sau phần khung thông tin chuẩn.
  void HienThiThongTin(ostream &os) const override {
    TaiLieu::HienThiThongTin(os); // Ra lệnh tải cột ban đầu
    // Biến kết gộp chuỗi giúp việc xếp cột bằng thuật sizeof không bị xô dích
    // UTF-8
    string thongTinRieng =
        "Số: " + to_string(soPhatHanh) + ", T" + to_string(thangPhatHanh);
    os << left << setw(8 + sizeof("Tạp chí") - 1) << "Tạp chí" << setw(20)
       << "N/A" << setw(11 + sizeof("Số: ") - 1) << thongTinRieng << YELLOW
       << setw(15) << fixed << setprecision(0) << TinhTien() << RESET << endl;
  }

  // Xếp giá bán quy chế Tạp chí theo hằng số 25,000 VNĐ một bản không đổi
  double TinhTien() const override { return 25000.0; }

  // Ghi đè phương thức lưu cho Tạp chí (Type 2)
  void Luu(ostream &os) const override {
    os << "2|";
    TaiLieu::Luu(os);
    os << "|" << soPhatHanh << "|" << thangPhatHanh << endl;
  }

  int getLoaiTaiLieu() const override { return 2; }

  void SuaThongTin() override {
    clearBuffer();
    cout << "--- SỬA THÔNG TIN TẠP CHÍ ---\n";
    TaiLieu::SuaThongTinChung();
    string tmp;
    cout << "Số phát hành (" << soPhatHanh
         << "). Nhập mới hoặc Enter để bỏ qua: ";
    getline(cin, tmp);
    if (!tmp.empty()) {
      try {
        soPhatHanh = stoi(tmp);
      } catch (...) {
      }
    }

    cout << "Tháng phát hành (" << thangPhatHanh
         << "). Nhập mới hoặc Enter để bỏ qua: ";
    getline(cin, tmp);
    if (!tmp.empty()) {
      try {
        thangPhatHanh = stoi(tmp);
      } catch (...) {
      }
    }
  }

  // Toán tử quá tải nhập/xuất riêng cho lớp Tạp chí (khớp với sơ đồ UML)
  // Quá tải toán tử luồng lệnh nhập chuẩn cin >> cho riêng Tạp Chí
  friend istream &operator>>(istream &is, TapChi &tc) {
    tc.Nhap(is, vector<TaiLieu *>());
    return is;
  }

  // Quá tải luồng in dòng cout << chuyên trách ráp bảng cho Tạp Chí
  friend ostream &operator<<(ostream &os, TapChi &tc) {
    tc.HienThiThongTin(os);
    return os;
  }
};

class Bao : public TaiLieu { // Gốc tạo đối tượng báo chí phát hành với thời hạn
private:
  int ngayPhatHanh; // Bổ sung thông tin đính kèm là ngày trong tháng

public:
  Bao() : TaiLieu(), ngayPhatHanh(0) {}
  Bao(string ma, string ten, string nxb, int nam, int nph)
      : TaiLieu(ma, ten, nxb, nam), ngayPhatHanh(nph) {}

  int getNgayPhatHanh() const { return ngayPhatHanh; }
  void setNgayPhatHanh(int nph) { ngayPhatHanh = nph; }

  // Xông chức năng Nhập của Báo đảm bảo thu hẹp mốc đầu vào của ngày trong
  // thang 1..31
  // Hàm Nhap (Được Ghi đè ở lớp con): Gọi nhờ hàm Nhập chung trước, kế tiếp yêu
  // cầu người dùng gõ thêm thông tin đặc thù.
  void Nhap(istream &is, const vector<TaiLieu *> &danhSach) override {
    TaiLieu::Nhap(is, danhSach); // Điền thông số chuẩn cho Báo

    // Tự động yêu cầu nhập đi lặp lại ngày cho tới khi thu được khung số quy
    // chuẩn hợp lệ
    do {
      if (is.fail()) { // Can thiệp reset bộ nhớ luồng hỏng
        is.clear();
        is.ignore(numeric_limits<streamsize>::max(), '\n');
      }
      cout << "Nhập ngày phát hành (1-31): ";
      is >> ngayPhatHanh;

      // Thông báo phản hồi nếu vạch lỗi vượt biên lớn hơn 31 hoặc bé hơn 1
      if (is.fail() || ngayPhatHanh < 1 || ngayPhatHanh > 31) {
        cout << RED
             << "Lỗi: Ngày phát hành phải từ 1 đến 31. Vui lòng nhập lại!"
             << RESET << endl;
      }
    } while (is.fail() || ngayPhatHanh < 1 ||
             ngayPhatHanh > 31); // Lệnh chốt khoá vòng nếu giá trị sạch
  }

  // Thực thi bản lề Hiện thị theo chiều ngang Báo và cột giá rỗng
  // Hàm HienThiThongTin (Được Ghi đè ở lớp con): Ráp nối các cột thông tin độc
  // quyền đằng sau phần khung thông tin chuẩn.
  void HienThiThongTin(ostream &os) const override {
    TaiLieu::HienThiThongTin(os); // Ra lệnh tải cột ban đầu
    // Đảo ngữ ghép string Ngày phát hành
    string thongTinRieng = "Ngày: " + to_string(ngayPhatHanh);
    os << left << setw(12 + sizeof("Báo") - 1) << "Báo" << setw(20) << "N/A"
       << setw(9 + sizeof("Ngày: ") - 1) << thongTinRieng << YELLOW << setw(15)
       << fixed << setprecision(0) << TinhTien() << RESET << endl;
  }

  // Tái cấu trúc giá trị tiền tệ của một quyển Báo lấy 5,000 VNĐ cố định
  double TinhTien() const override { return 5000.0; }

  // Ghi đè phương thức lưu cho Báo (Type 3)
  void Luu(ostream &os) const override {
    os << "3|";
    TaiLieu::Luu(os);
    os << "|" << ngayPhatHanh << endl;
  }

  int getLoaiTaiLieu() const override { return 3; }

  void SuaThongTin() override {
    clearBuffer();
    cout << "--- SỬA THÔNG TIN BÁO ---\n";
    TaiLieu::SuaThongTinChung();
    string tmp;
    cout << "Ngày phát hành (" << ngayPhatHanh
         << "). Nhập mới hoặc Enter để bỏ qua: ";
    getline(cin, tmp);
    if (!tmp.empty()) {
      try {
        ngayPhatHanh = stoi(tmp);
      } catch (...) {
      }
    }
  }

  // Toán tử quá tải nhập/xuất riêng cho lớp Báo (khớp với sơ đồ UML)
  // Quá tải luồng chuẩn nhập dữ liệu cin >> áp dụng đối tượng Báo
  friend istream &operator>>(istream &is, Bao &b) {
    b.Nhap(is, vector<TaiLieu *>());
    return is;
  }

  // Toán tử cho phép cout tự do hiển thị dòng chữ của đối tượng Báo không qua
  // hàm
  friend ostream &operator<<(ostream &os, Bao &b) {
    b.HienThiThongTin(os);
    return os;
  }
};

// ==========================================================
// 3. HỆ THỐNG QUẢN LÝ (LibrarySystem)
// ==========================================================

// Hàm vẽ khung đỉnh và đề mục bảng hiển thị danh sách chi tiết (xử lý byte bù
// trừ cho UTF-8 an toàn)
// Hàm InTieuDeBang: (Toàn cục) Giúp xây dựng cấu trúc in bảng vạch kẻ và giãn
// khoảng cách (setw) hợp lý
// === CÁC HÀM XỬ LÝ FILE (Persistence) ===
// Hàm tiện ích chuyển sang chữ thường (Case-insensitive)
// Hàm toLowerCase: Trợ thủ biến toàn bộ string chữ in hoa sang kiểu chữ in
// thường, rất hữu dụng khi tìm tài liệu
// ==========================================================
// 3. LỚP QUẢN LÝ THƯ VIỆN: LibrarySystem
// ==========================================================
// Giải thích: Lớp Quản lý trung tâm, đóng gói (Encapsulate) toàn bộ dữ liệu và
// các hàm nghiệp vụ của chương trình.
// Giải thích: Mảng động chứa danh sách tài liệu. Nhờ Đa hình, 1 mảng này có
// thể chứa mix cả Sách, Báo và Tạp chí.
// Hàm LoadDuLieu: Đọc dữ liệu từ file txt lên bộ nhớ (vào mảng) khi vừa khởi
// động chương trình.
// Dùng công cụ stringstream cắt xẻ chuỗi trên một dòng dựa trên dấu vạch
// đứng '|'
// Ép kiểu chuỗi sang số nguyên để xác định loại: 1 (Sách), 2 (Tạp Chí),
// 3 (Báo)
// Hàm LuuDuLieu: Đồng bộ/ghi lưu toàn bộ danh sách tài liệu hiện có trong
// mảng xuống file data.txt.
// Hàm thongKeChiTiet: Đếm số lượng, tính tổng tiền và hiển thị thống kê tổng
// quan các loại tài liệu.
// Gộp tất cả các biến cộng đồn lại để chốt thông số tổng
// Hàm timKiemTaiLieu: Hỗ trợ tìm kiếm nhanh tài liệu theo Mã hoặc 1 phần Tên
// (không phân biệt viết hoa/thường).
// Ép toàn bộ từ khóa tìm kiếm sang chữ viết thường để chuẩn hóa độ chính
// xác
// Kích hoạt ghép khớp nếu từ khóa trùng tuyệt đối mã, HOẶC nằm xen giữa
// tên tài liệu
// Hàm muonTaiLieu: Trích chuyển trạng thái tài liệu sang 'Đang mượn' và khóa
// lại bằng tên người mượn.
// Khóa cờ đánh dấu là đã bị mang đi
// Gắn biên lai tên người dùng đang mượn trực tiếp vào tài liệu
// Hàm traTaiLieu: Đổi trạng thái tái lưu trữ thành sẵn sàng và gỡ bỏ tên
// người mượn.
// Hàm xoaTaiLieu: Giải phóng hoàn toàn RAM (heap) của tài liệu và đá văng
// khỏi danh sách (chặn xóa nếu đang có người mượn).
// Tạo một mảng phụ quét gom những vị trí có nhiều bản sao trùng mã với nhau
// Lệnh xóa: Phá hủy thu hồi hoàn toàn khoảng nhớ RAM tĩnh
// Cắt đứt mắt xích chứa tài liệu này khỏi bộ khung danh sách chính
// Giải thích: Nơi chương trình bắt đầu chạy, chứa vòng lặp hiển thị Menu Tương
// tác cho người dùng.
// Kích hoạt Lõi Hệ thống Thư viện tổng (Gom toàn thủ tục vận hành và RAM vào
// đây)
// Rào chắn bảo mật: Buộc phải xác thực Account đi qua mới được vào lõi giao
// tiếp
// Khai báo biến lưu giữ phím số người dùng vừa bấm để gọi Trình đơn (Menu)
class LibrarySystem {
private:
  vector<TaiLieu *> ds;

  void InTieuDeBang() {
    cout << string(155, '-') << endl;
    cout << BOLD << CYAN << left << setw(5 + sizeof("Mã TL") - 1) << "Mã TL"
         << setw(13 + sizeof("Tên Tài Liệu") - 1) << "Tên Tài Liệu"
         << setw(8 + sizeof("Nhà Xuất Bản") - 1) << "Nhà Xuất Bản"
         << setw(4 + sizeof("Năm XB") - 1) << "Năm XB"
         << setw(15 + sizeof("Tình Trạng") - 1) << "Tình Trạng"
         << setw(6 + sizeof("Phân Loại") - 1) << "Phân Loại"
         << setw(13 + sizeof("Tác Giả") - 1) << "Tác Giả"
         << setw(7 + sizeof("Chi Tiết") - 1) << "Chi Tiết"
         << setw(7 + sizeof("Giá Tiền") - 1) << "Giá Tiền" << RESET << endl;
    cout << string(155, '-') << endl;
  }

  void xuatDanhSachCSV(const vector<TaiLieu *> &dsToPrint) {
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", &tstruct);
    string filename = "export_" + string(buf) + ".csv";

    ofstream ofs(filename);
    ofs << "MaTL,TenTaiLieu,NhaXuatBan,NamXB,TinhTrang,Loai,ChiTiet\n";
    for (auto tl : dsToPrint) {
      if (tl) {
        string loai = (tl->getLoaiTaiLieu() == 1)   ? "Sach"
                      : (tl->getLoaiTaiLieu() == 2) ? "TapChi"
                                                    : "Bao";
        ofs << tl->getMaTaiLieu() << "," << tl->getTenTaiLieu() << ","
            << tl->getNhaXuatBan() << "," << tl->getNamXuatBan() << ","
            << (tl->getIsBorrowed() ? "Dang muon" : "San sang") << "," << loai
            << "," << tl->getChiTietHienThi() << "\n";
      }
    }
    cout << GREEN << "Đã xuất dữ liệu ra file " << filename << "\n" << RESET;
  }

  string toLowerCase(string s) {
    transform(s.begin(), s.end(), s.begin(),
              [](unsigned char c) { return tolower(c); });
    return s;
  }

public:
  LibrarySystem() {}

  // Tùy theo chức vụ mà khởi động luồng logic xử lý case (trường hợp) tương
  // ứng
  ~LibrarySystem() {
    for (auto tl : ds) {
      if (tl != nullptr) {
        delete tl;
      }
    }
    ds.clear();
  }

  void LoadDuLieu() {
    ifstream ifs("data.txt");
    if (!ifs.is_open())
      return;

    string line;
    while (getline(ifs, line)) {
      if (line.empty())
        continue;
      stringstream ss(line);
      string typeStr, ma, ten, nxb, namStr, borrowStr, borrower;

      // Tùy theo chức vụ mà khởi động luồng logic xử lý case (trường hợp) tương
      // ứng
      getline(ss, typeStr, '|');
      getline(ss, ma, '|');
      getline(ss, ten, '|');
      getline(ss, nxb, '|');
      getline(ss, namStr, '|');
      getline(ss, borrowStr, '|');
      getline(ss, borrower, '|');

      if (typeStr.empty())
        continue;

      try {
        int type = stoi(typeStr);
        TaiLieu *newItem = nullptr;

        if (type == 1) {
          string stStr, tacGia;
          getline(ss, stStr, '|');
          getline(ss, tacGia, '|');
          newItem = new Sach(ma, ten, nxb, stoi(namStr), stoi(stStr), tacGia);
        } else if (type == 2) {
          string sphStr, tphStr;
          getline(ss, sphStr, '|');
          getline(ss, tphStr, '|');
          newItem = new TapChi(ma, ten, nxb, stoi(namStr), stoi(sphStr),
                               stoi(tphStr));
        } else if (type == 3) {
          string nphStr;
          getline(ss, nphStr, '|');
          newItem = new Bao(ma, ten, nxb, stoi(namStr), stoi(nphStr));
        }

        if (newItem != nullptr) {
          newItem->setIsBorrowed(borrowStr == "1");
          newItem->setBorrowerName(borrower);
          ds.push_back(newItem);
        }
      } catch (...) {
        continue;
      }
    }
    ifs.close();
  }

  void LuuDuLieu() {
    ofstream ofs("data.txt");
    if (ofs.is_open()) {
      for (const auto &tl : ds) {
        if (tl != nullptr) {
          tl->Luu(ofs);
        }
      }
      ofs.close();
    }
  }

  void thongKeChiTiet() {
    int soSach = 0, soTapChi = 0, soBao = 0;
    int sachDangMuon = 0, tapChiDangMuon = 0, baoDangMuon = 0;
    double tienSach = 0, tienTapChi = 0, tienBao = 0;

    for (const auto &tl : ds) {
      if (tl == nullptr)
        continue;
      int type = tl->getLoaiTaiLieu();
      if (type == 1) {
        soSach++;
        tienSach += tl->TinhTien();
        if (tl->getIsBorrowed())
          sachDangMuon++;
      } else if (type == 2) {
        soTapChi++;
        tienTapChi += tl->TinhTien();
        if (tl->getIsBorrowed())
          tapChiDangMuon++;
      } else if (type == 3) {
        soBao++;
        tienBao += tl->TinhTien();
        if (tl->getIsBorrowed())
          baoDangMuon++;
      }
    }

    int tong = soSach + soTapChi + soBao;
    int tongDangMuon = sachDangMuon + tapChiDangMuon + baoDangMuon;
    double tongTien = tienSach + tienTapChi + tienBao;

    cout << "\n"
         << BOLD << CYAN << "========== THỐNG KÊ CHI TIẾT ==========" << RESET
         << endl;
    cout << BOLD << GREEN << "\n--- SÁCH ---\n" << RESET;
    cout << "Tổng số: " << soSach << "\nĐang mượn: " << sachDangMuon
         << "\nCòn lại: " << (soSach - sachDangMuon) << "\nTổng tiền: " << fixed
         << setprecision(0) << tienSach << " VNĐ\n";

    cout << BOLD << GREEN << "\n--- TẠP CHÍ ---\n" << RESET;
    cout << "Tổng số: " << soTapChi << "\nĐang mượn: " << tapChiDangMuon
         << "\nCòn lại: " << (soTapChi - tapChiDangMuon)
         << "\nTổng tiền: " << fixed << setprecision(0) << tienTapChi
         << " VNĐ\n";

    cout << BOLD << GREEN << "\n--- BÁO ---\n" << RESET;
    cout << "Tổng số: " << soBao << "\nĐang mượn: " << baoDangMuon
         << "\nCòn lại: " << (soBao - baoDangMuon) << "\nTổng tiền: " << fixed
         << setprecision(0) << tienBao << " VNĐ\n";

    cout << BOLD << RED << "\n=== TỔNG TOÀN BỘ ===\n" << RESET;
    cout << "Tổng tài liệu: " << tong << "\nTổng đang mượn: " << tongDangMuon
         << "\nTổng còn lại: " << (tong - tongDangMuon)
         << "\nTổng tiền tất cả: " << fixed << setprecision(0) << tongTien
         << " VNĐ\n";
  }

  void xemVaLocDanhSach() {
    if (ds.empty()) {
      cout << RED << "Danh sách trống!\n" << RESET;
      return;
    }

    int filterOption = 0;
    cout << "\nBộ lọc: 1. Tất cả | 2. Sách | 3. Tạp chí | 4. Báo\nChọn: ";
    cin >> filterOption;
    clearBuffer();

    int sortOption = 0;
    cout << "Sắp xếp: 1. Không | 2. Theo Tên (A-Z) | 3. Theo Năm Xuất Bản | 4. "
            "Theo Giá\nChọn: ";
    cin >> sortOption;
    clearBuffer();

    vector<TaiLieu *> dsToPrint;
    for (auto tl : ds) {
      if (filterOption == 1 ||
          (filterOption == 2 && tl->getLoaiTaiLieu() == 1) ||
          (filterOption == 3 && tl->getLoaiTaiLieu() == 2) ||
          (filterOption == 4 && tl->getLoaiTaiLieu() == 3)) {
        dsToPrint.push_back(tl);
      }
    }

    if (sortOption == 2) {
      sort(dsToPrint.begin(), dsToPrint.end(), [this](TaiLieu *a, TaiLieu *b) {
        return toLowerCase(a->getTenTaiLieu()) <
               toLowerCase(b->getTenTaiLieu());
      });
    } else if (sortOption == 3) {
      sort(dsToPrint.begin(), dsToPrint.end(), [](TaiLieu *a, TaiLieu *b) {
        return a->getNamXuatBan() < b->getNamXuatBan();
      });
    } else if (sortOption == 4) {
      sort(dsToPrint.begin(), dsToPrint.end(), [](TaiLieu *a, TaiLieu *b) {
        return a->TinhTien() < b->TinhTien();
      });
    }

    if (dsToPrint.empty()) {
      cout << YELLOW << "Không có tài liệu nào phù hợp bộ lọc.\n" << RESET;
      return;
    }

    cout << "\n" << BOLD << GREEN << "DANH SÁCH TÀI LIỆU" << RESET << "\n";
    InTieuDeBang();
    double tongTien = 0;
    for (const auto &tl : dsToPrint) {
      tl->HienThiThongTin(cout);
      tongTien += tl->TinhTien();
    }
    cout << string(155, '-') << endl;
    cout << BOLD << RED << "TỔNG TIỀN TÀI LIỆU ĐANG HIỂN THỊ: " << fixed
         << setprecision(0) << tongTien << " VNĐ" << RESET << endl;

    char exportChoice;
    cout << "Bạn có muốn xuất danh sách này ra file CSV không? (Y/N): ";
    cin >> exportChoice;
    clearBuffer();
    if (exportChoice == 'Y' || exportChoice == 'y') {
      xuatDanhSachCSV(dsToPrint);
    }
  }

  void timKiemTaiLieu() {
    string keyword;
    cout << "Nhập mã, tên tài liệu, NXB hoặc tác giả cần tìm: ";
    getline(cin, keyword);

    string keywordToLower = toLowerCase(keyword);

    bool found = false;
    for (const auto &tl : ds) {
      if (tl != nullptr) {
        string maToLower = toLowerCase(tl->getMaTaiLieu());
        string tenToLower = toLowerCase(tl->getTenTaiLieu());
        string nxbToLower = toLowerCase(tl->getNhaXuatBan());
        string chiTietToLower = toLowerCase(tl->getChiTietHienThi());

        if (maToLower == keywordToLower ||
            tenToLower.find(keywordToLower) != string::npos ||
            nxbToLower.find(keywordToLower) != string::npos ||
            chiTietToLower.find(keywordToLower) != string::npos) {
          if (!found) {
            cout << "\n"
                 << BOLD << GREEN << "KẾT QUẢ TÌM KIẾM" << RESET << "\n";
            InTieuDeBang();
            found = true;
          }
          tl->HienThiThongTin(cout);
        }
      }
    }
    if (!found)
      cout << RED << "Không tìm thấy tài liệu!" << RESET << "\n";
    else
      cout << string(155, '-') << endl;
  }

  void muonTaiLieu(User *currentUser) {
    if (currentUser->getRole() != "Reader") {
      cout << RED << "Chỉ Reader mới có quyền mượn tài liệu!" << RESET << "\n";
      return;
    }

    int activeBorrows = 0;
    for (const auto &tb : ds) {
      if (tb != nullptr && tb->getIsBorrowed() &&
          tb->getBorrowerName() == currentUser->getUsername()) {
        activeBorrows++;
      }
    }
    if (activeBorrows >= 3) {
      cout << RED
           << "Lỗi: Bạn đã đạt giới hạn mượn tối đa 3 tài liệu cùng lúc.\n"
           << RESET;
      return;
    }

    string ma;
    cout << "Nhập mã tài liệu muốn mượn: ";
    cin >> ma;
    clearBuffer();
    string maToLower = toLowerCase(ma);

    for (auto &tl : ds) {
      if (tl != nullptr && toLowerCase(tl->getMaTaiLieu()) == maToLower) {
        if (tl->getIsBorrowed()) {
          cout << RED << "Tài liệu này đã được mượn bởi "
               << tl->getBorrowerName() << "!" << RESET << "\n";
        } else {
          tl->setIsBorrowed(true);
          tl->setBorrowerName(currentUser->getUsername());
          LuuDuLieu();
          ghiLichSu(currentUser->getUsername(), "Mượn", tl->getMaTaiLieu(),
                    tl->getTenTaiLieu());
          cout << GREEN << "Mượn tài liệu thành công!" << RESET << "\n";
        }
        return;
      }
    }
    cout << RED << "Không tìm thấy mã tài liệu!" << RESET << "\n";
  }

  void traTaiLieu(User *currentUser) {
    if (currentUser->getRole() != "Reader") {
      cout << RED << "Chỉ Reader mới có quyền trả tài liệu!" << RESET << "\n";
      return;
    }
    string ma;
    cout << "Nhập mã tài liệu muốn trả: ";
    cin >> ma;
    clearBuffer();
    string maToLower = toLowerCase(ma);

    for (auto &tl : ds) {
      if (tl != nullptr && toLowerCase(tl->getMaTaiLieu()) == maToLower) {
        if (!tl->getIsBorrowed()) {
          cout << YELLOW << "Tài liệu này đang rảnh (chưa ai mượn)." << RESET
               << "\n";
        } else if (tl->getBorrowerName() != currentUser->getUsername()) {
          cout << RED << "Bạn không thể trả tài liệu do người khác ("
               << tl->getBorrowerName() << ") mượn!" << RESET << "\n";
        } else {
          tl->setIsBorrowed(false);
          tl->setBorrowerName("");
          LuuDuLieu();
          ghiLichSu(currentUser->getUsername(), "Trả", tl->getMaTaiLieu(),
                    tl->getTenTaiLieu());
          cout << GREEN << "Trả tài liệu thành công!" << RESET << "\n";
        }
        return;
      }
    }
    cout << RED << "Không tìm thấy mã tài liệu!" << RESET << "\n";
  }

  void ghiLichSu(string username, string action, string maTL, string tenTL) {
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);

    ofstream ofs("history.txt", ios::app);
    if (ofs) {
      ofs << username << "|" << action << "|" << maTL << "|" << tenTL << "|"
          << buf << "\n";
      ofs.close();
    }
  }

  void xemLichSu(string username = "") {
    ifstream ifs("history.txt");
    if (!ifs.is_open()) {
      cout << RED << "Chưa có lịch sử giao dịch nào.\n" << RESET;
      return;
    }

    cout << "\n"
         << BOLD << GREEN << "=== LỊCH SỬ GIAO DỊCH ===" << RESET << "\n";
    cout << left << setw(15) << "Người dùng" << setw(15) << "Hành động"
         << setw(15) << "Mã TL" << setw(30) << "Tên TL" << "Thời gian" << "\n";
    cout << string(100, '-') << "\n";

    string line;
    bool found = false;
    while (getline(ifs, line)) {
      stringstream ss(line);
      string un, act, ma, ten, timeStr;
      getline(ss, un, '|');
      getline(ss, act, '|');
      getline(ss, ma, '|');
      getline(ss, ten, '|');
      getline(ss, timeStr, '|');

      if (username == "" || un == username) {
        cout << left << setw(15) << un << setw(15) << act << setw(15) << ma
             << setw(30) << ten << timeStr << "\n";
        found = true;
      }
    }
    ifs.close();
    if (!found)
      cout << YELLOW << "Không có giao dịch nào phù hợp.\n" << RESET;
  }

  void themTaiLieu(int loai) {
    TaiLieu *tlMoi = nullptr;
    if (loai == 1) {
      cout << CYAN << "\nNhập thông tin Sách" << RESET << endl;
      tlMoi = new Sach();
    } else if (loai == 2) {
      cout << CYAN << "\nNhập thông tin Tạp chí" << RESET << endl;
      tlMoi = new TapChi();
    } else if (loai == 3) {
      cout << CYAN << "\nNhập thông tin Báo" << RESET << endl;
      tlMoi = new Bao();
    }

    if (tlMoi != nullptr) {
      tlMoi->Nhap(cin, ds);
      ds.push_back(tlMoi);
      cout << GREEN << "Thêm tài liệu thành công!\n" << RESET;
      LuuDuLieu();
    }
  }

  // Tạo vỏ bọc mới cấp phát trên RAM dọn chỗ chuẩn bị chứa Sách
  // Đẩy nhồi dữ liệu vỏ bọc vừa rồi ghép nối vào toa tàu cấp cao (Danh
  // sách chung)
  // Giải phóng thông tin ca gác cũ lúc bị ấn 0 thoát hệ thống
  void xoaTaiLieu() {
    if (ds.empty()) {
      cout << RED << "Danh sách trống" << RESET << endl;
      return;
    }
    string maCanXoa;
    cout << "Nhập mã tài liệu cần xóa: ";
    cin >> maCanXoa;
    clearBuffer();
    string maToLower = toLowerCase(maCanXoa);

    vector<int> viTriXoa;
    for (int i = 0; i < (int)ds.size(); i++) {
      if (toLowerCase(ds[i]->getMaTaiLieu()) == maToLower) {
        viTriXoa.push_back(i);
      }
    }

    if (viTriXoa.empty()) {
      cout << RED << "Không tìm thấy mã tài liệu \"" << maCanXoa << "\"!"
           << RESET << endl;
      return;
    }

    int index = -1;
    cout << "\n"
         << BOLD << GREEN << "TÀI LIỆU CÓ MÃ \"" << maCanXoa << "\"" << RESET
         << "\n";
    InTieuDeBang();

    if (viTriXoa.size() == 1) {
      index = viTriXoa[0];
      ds[index]->HienThiThongTin(cout);
      cout << endl;
    } else {
      for (int i = 0; i < (int)viTriXoa.size(); i++) {
        cout << BOLD << YELLOW << "Lựa chọn số " << (i + 1) << ":" << RESET
             << endl;
        ds[viTriXoa[i]]->HienThiThongTin(cout);
        cout << endl;
      }
      cout << string(155, '-') << endl;
      int luaChonXoa;
      bool hopLe = false;
      while (!hopLe) {
        cout << "Có nhiều tài liệu cùng mã. Nhập lựa chọn số (1-"
             << viTriXoa.size() << "): ";
        if (!(cin >> luaChonXoa) || luaChonXoa < 1 ||
            luaChonXoa > (int)viTriXoa.size()) {
          clearBuffer();
          cout << RED << "Lựa chọn không hợp lệ. Vui lòng nhập lại!" << RESET
               << endl;
        } else {
          clearBuffer();
          hopLe = true;
        }
      }
      index = viTriXoa[luaChonXoa - 1];
      cout << GREEN << "Tài liệu được chọn: " << RESET << endl;
      InTieuDeBang();
      ds[index]->HienThiThongTin(cout);
      cout << endl;
    }
    cout << string(155, '-') << endl;

    if (ds[index]->getIsBorrowed()) {
      cout << BOLD << RED << "LỖI: Trạng thái hiện tại là 'Đang mượn'." << endl;
      cout << "Tài liệu đang được mượn bởi " << ds[index]->getBorrowerName()
           << ". Không thể xóa!" << RESET << endl;
      return;
    }

    char xacNhan;
    cout << BOLD << RED
         << "Bạn có chắc chắn muốn xóa tài liệu này? (Y/N): " << RESET;
    cin >> xacNhan;
    clearBuffer();
    if (xacNhan == 'Y' || xacNhan == 'y') {
      delete ds[index];
      ds.erase(ds.begin() + index);
      LuuDuLieu();
      cout << GREEN << "Xóa tài liệu thành công!" << RESET << endl;
    } else {
      cout << YELLOW << "Hủy xóa tài liệu." << RESET << endl;
    }
  }

  void suaThongTinTaiLieu() {
    if (ds.empty()) {
      cout << RED << "Danh sách trống" << RESET << endl;
      return;
    }
    string maCanSua;
    cout << "Nhập mã tài liệu cần sửa: ";
    cin >> maCanSua;
    clearBuffer();
    string maToLower = toLowerCase(maCanSua);
    bool modified = false;
    for (auto &tl : ds) {
      if (tl != nullptr && toLowerCase(tl->getMaTaiLieu()) == maToLower) {
        tl->SuaThongTin();
        modified = true;
      }
    }
    if (modified) {
      LuuDuLieu();
      cout << GREEN << "Sửa thông tin thành công!" << RESET << endl;
    } else {
      cout << RED << "Không tìm thấy mã tài liệu!" << RESET << endl;
    }
  }
};

// ==========================================================
// 4. CHƯƠNG TRÌNH CHÍNH (Vận hành trung tâm Console)
// ==========================================================

int main() {
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);

  LibrarySystem lib;
  lib.LoadDuLieu();

  while (true) {
    User *currentUser = AuthManager::login();
    if (currentUser == nullptr) {
      cout << "Đã hủy đăng nhập.\n";
      break;
    }

    int luaChon;
    do {
      cout << "\n"
           << BOLD << GREEN << "=== HỆ THỐNG QUẢN LÝ THƯ VIỆN ===" << RESET
           << "\n";
      cout << CYAN << "Xin chào, " << currentUser->getUsername() << " ("
           << currentUser->getRole() << ")\n"
           << RESET;

      if (currentUser->getRole() == "Librarian") {
        cout << "1. Thêm Sách\n2. Thêm Tạp Chí\n3. Thêm Báo\n4. Hiển thị danh "
                "sách và Tính tiền\n";
        cout << "5. Thống kê chi tiết\n6. Xóa tài liệu\n7. Đổi mật khẩu\n8. "
                "Xem tất cả lịch sử\n9. Sửa thông tin tài liệu\n0. Thoát và "
                "Đăng xuất\n";
      } else {
        cout << "1. Tìm kiếm tài liệu\n2. Mượn tài liệu\n3. Trả tài liệu\n4. "
                "Xem danh sách tài liệu\n";
        cout << "5. Đổi mật khẩu\n6. Xem lịch sử của tôi\n0. Thoát và Đăng "
                "xuất\n";
      }
      cout << "Nhập lựa chọn của bạn: ";

      if (!(cin >> luaChon)) {
        cout << RED << "Lỗi: Lựa chọn không hợp lệ!\n" << RESET;
        clearBuffer();
        luaChon = -1;
        continue;
      }

      if (currentUser->getRole() == "Librarian") {
        switch (luaChon) {
        case 1:
          lib.themTaiLieu(1);
          break;
        case 2:
          lib.themTaiLieu(2);
          break;
        case 3:
          lib.themTaiLieu(3);
          break;
        case 4:
          lib.xemVaLocDanhSach();
          break;
        case 5:
          lib.thongKeChiTiet();
          break;
        case 6:
          lib.xoaTaiLieu();
          break;
        case 7:
          AuthManager::doiMatKhau(currentUser);
          break;
        case 8:
          lib.xemLichSu();
          break;
        case 9:
          lib.suaThongTinTaiLieu();
          break;
        case 0:
          cout << "Đang đăng xuất...\n";
          break;
        default:
          cout << RED << "Lựa chọn không hợp lệ. Vui lòng nhập lại!\n" << RESET;
          clearBuffer();
        }
      } else {
        switch (luaChon) {
        case 1:
          clearBuffer();
          lib.timKiemTaiLieu();
          break;
        case 2:
          lib.muonTaiLieu(currentUser);
          break;
        case 3:
          lib.traTaiLieu(currentUser);
          break;
        case 4:
          lib.xemVaLocDanhSach();
          break;
        case 5:
          AuthManager::doiMatKhau(currentUser);
          break;
        case 6:
          lib.xemLichSu(currentUser->getUsername());
          break;
        case 0:
          cout << "Đang đăng xuất...\n";
          break;
        default:
          cout << RED << "Lựa chọn không hợp lệ. Vui lòng nhập lại!\n" << RESET;
          clearBuffer();
        }
      }
    } while (luaChon != 0);

    delete currentUser;
  }

  return 0;
}
