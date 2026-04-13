// Nạp các thư viện chuẩn cần thiết cho chương trình
#include <algorithm> // Thư viện chứa std::transform
#include <cctype>    // Thư viện chứa ::tolower
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
};

// Lớp Librarian (Thủ thư): Đại diện cho nhân viên thư viện, có các quyền quản lý đầy đủ (Thêm/Xóa)
class Librarian : public User {
public:
  Librarian(string uname, string pwd) : User(uname, pwd, "Librarian") {}
};

// Lớp Reader (Độc giả): Đại diện cho người dùng bình thường, chỉ có quyền Mượn/Trả/Tìm kiếm
class Reader : public User {
public:
  Reader(string uname, string pwd) : User(uname, pwd, "Reader") {}
};

// Lớp AuthManager (Quản lý Xác thực): Đảm nhận vai trò kiểm tra đăng nhập và quản lý danh sách tài khoản
class AuthManager {
public:
  // Hàm taoFileChuan: Tạo ra file db account mặc định nếu hệ thống chưa có sẵn file users.txt
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

  // Hàm login: Tạo vòng lặp yêu cầu người dùng nhập tên và mật khẩu. Trả về đúng đối tượng User theo quyền nếu khớp file.
  static User *login() {
    taoFileChuan();
    string reqUser, reqPwd;
    while (true) {
      cout << "\n"
           << BOLD << CYAN << "=== ĐĂNG NHẬP HỆ THỐNG ===" << RESET << "\n";
      cout << "Tài khoản (Nhập '0' để thoát): ";
      cin >> reqUser;
      if (reqUser == "0")
        return nullptr;
      cout << "Mật khẩu: ";
      cin >> reqPwd;

      ifstream ifs("users.txt");
      string line;
      while (getline(ifs, line)) {
        // Dùng công cụ stringstream cắt xẻ chuỗi trên một dòng dựa trên dấu
        // vạch đứng '|'
        stringstream ss(line);
        string uname, pwd, role;
        getline(ss, uname, '|');
        getline(ss, pwd, '|');
        getline(ss, role, '|');

        if (uname == reqUser && pwd == reqPwd) {
          cout << GREEN << "\nĐăng nhập thành công! Vai trò: " << role << RESET
               << "\n";
          if (role == "Librarian")
            return new Librarian(uname, pwd);
          else
            return new Reader(uname, pwd);
        }
      }
      cout << RED << "Tài khoản hoặc mật khẩu không chính xác. Thử lại!"
           << RESET << "\n";
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
  // Bộ Getter/Setter: Dùng để an toàn lấy (get) hoặc ghi đè (set) dữ liệu riêng tư từ bên ngoài lớp
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
       << TinhTien() << RESET << endl;
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

  // Toán tử quá tải nhập/xuất riêng cho lớp Sách (khớp với sơ đồ UML)
  // Quá tải toán tử nhúng (>>): Cho phép lệnh cin tác động trực tiếp lên nguyên một khối Sách s
  friend istream &operator>>(istream &is, Sach &s) {
    // Lưu ý: operator>> không có truy cập vào danhSach để check duy nhất,
    // nên khuyến khích dùng Nhap(is, danhSach) trực tiếp.
    s.Nhap(is, vector<TaiLieu *>());
    return is;
  }

  // Quá tải toán tử in (<<): Giúp lệnh cout in thẳng đối tượng Sách s thay vì phải gọi s.HienThiThongTin()
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
       << setw(15) << TinhTien() << RESET << endl;
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
       << TinhTien() << RESET << endl;
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

  // Toán tử quá tải nhập/xuất riêng cho lớp Báo (khớp với sơ đồ UML)
  // Quá tải luồng chuẩn nhập dữ liệu cin >> áp dụng đối tượng Báo
  friend istream &operator>>(istream &is, Bao &b) {
    b.Nhap(is, vector<TaiLieu *>());
    return is;
  }

  // Toán tử cho phép cout tự do hiển thị dòng chữ của đối tượng Báo không qua hàm
  friend ostream &operator<<(ostream &os, Bao &b) {
    b.HienThiThongTin(os);
    return os;
  }
};

// ==========================================================
// 3. CHƯƠNG TRÌNH CHÍNH (Vận hành trung tâm Console)
// ==========================================================

// Hàm vẽ khung đỉnh và đề mục bảng hiển thị danh sách chi tiết (xử lý byte bù
// trừ cho UTF-8 an toàn)
// Hàm InTieuDeBang: (Toàn cục) Giúp xây dựng cấu trúc in bảng vạch kẻ và giãn khoảng cách (setw) hợp lý
void InTieuDeBang() {
  cout << string(155, '-')
       << endl; // Đổ ra một dải đường ngang biên giới trên độ dài 155 ký hiệu
  cout << BOLD << CYAN << left << setw(5 + sizeof("Mã TL") - 1) << "Mã TL"
       << setw(13 + sizeof("Tên Tài Liệu") - 1) << "Tên Tài Liệu"
       << setw(8 + sizeof("Nhà Xuất Bản") - 1) << "Nhà Xuất Bản"
       << setw(4 + sizeof("Năm XB") - 1) << "Năm XB"
       << setw(15 + sizeof("Tình Trạng") - 1) << "Tình Trạng"
       << setw(6 + sizeof("Phân Loại") - 1) << "Phân Loại"
       << setw(13 + sizeof("Tác Giả") - 1) << "Tác Giả"
       << setw(7 + sizeof("Chi Tiết") - 1) << "Chi Tiết"
       << setw(7 + sizeof("Giá Tiền") - 1) << "Giá Tiền" << RESET
       << endl; // Chèn mã kết thúc làm màu về mặc định
  cout << string(155, '-')
       << endl; // Đổ đường viền phân tách các hàng thông tin số liệu chính yếu
}

// === CÁC HÀM XỬ LÝ FILE (Persistence) ===
// Hàm tiện ích chuyển sang chữ thường (Case-insensitive)
// Hàm toLowerCase: Trợ thủ biến toàn bộ string chữ in hoa sang kiểu chữ in thường, rất hữu dụng khi tìm tài liệu
string toLowerCase(string s) {
  transform(s.begin(), s.end(), s.begin(),
            [](unsigned char c) { return tolower(c); });
  return s;
}

// ==========================================================
// 3. LỚP QUẢN LÝ THƯ VIỆN: LibrarySystem
// ==========================================================
// Giải thích: Lớp Quản lý trung tâm, đóng gói (Encapsulate) toàn bộ dữ liệu và
// các hàm nghiệp vụ của chương trình.
class LibrarySystem {
private:
  // Giải thích: Mảng động chứa danh sách tài liệu. Nhờ Đa hình, 1 mảng này có
  // thể chứa mix cả Sách, Báo và Tạp chí.
  vector<TaiLieu *> ds;

public:
  LibrarySystem() {}

  ~LibrarySystem() {
    for (auto tl : ds) {
      if (tl != nullptr) {
        delete tl;
      }
    }
    ds.clear();
  }

  vector<TaiLieu *> &getDS() { return ds; }

  // Hàm LoadDuLieu: Đọc dữ liệu từ file txt lên bộ nhớ (vào mảng) khi vừa khởi
  // động chương trình.
  void LoadDuLieu(vector<TaiLieu *> &danhSach) {
    ifstream ifs("data.txt");
    if (!ifs.is_open())
      return;

    string line;
    while (getline(ifs, line)) {
      if (line.empty())
        continue;
      // Dùng công cụ stringstream cắt xẻ chuỗi trên một dòng dựa trên dấu vạch
      // đứng '|'
      stringstream ss(line);
      string typeStr, ma, ten, nxb, namStr, borrowStr, borrower;

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
        // Ép kiểu chuỗi sang số nguyên để xác định loại: 1 (Sách), 2 (Tạp Chí),
        // 3 (Báo)
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
          danhSach.push_back(newItem);
        }
      } catch (...) {
        continue;
      }
    }
    ifs.close();
  }

  // Hàm LuuDuLieu: Đồng bộ/ghi lưu toàn bộ danh sách tài liệu hiện có trong
  // mảng xuống file data.txt.
  void LuuDuLieu(const vector<TaiLieu *> &danhSach) {
    ofstream ofs("data.txt");
    if (ofs.is_open()) {
      for (const auto &tl : danhSach) {
        if (tl != nullptr) {
          tl->Luu(ofs);
        }
      }
      ofs.close();
    }
  }

  // Hàm thongKeChiTiet: Đếm số lượng, tính tổng tiền và hiển thị thống kê tổng
  // quan các loại tài liệu.
  void thongKeChiTiet(const vector<TaiLieu *> &danhSach) {
    int soSach = 0, soTapChi = 0, soBao = 0;
    int sachDangMuon = 0, tapChiDangMuon = 0, baoDangMuon = 0;
    double tienSach = 0, tienTapChi = 0, tienBao = 0;

    for (const auto &tl : danhSach) {
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

    // Gộp tất cả các biến cộng đồn lại để chốt thông số tổng
    int tong = soSach + soTapChi + soBao;
    int tongDangMuon = sachDangMuon + tapChiDangMuon + baoDangMuon;
    double tongTien = tienSach + tienTapChi + tienBao;

    cout << "\n"
         << BOLD << CYAN << "========== THỐNG KÊ CHI TIẾT ==========" << RESET
         << endl;

    cout << BOLD << GREEN << "\n--- SÁCH ---\n" << RESET;
    cout << "Tổng số: " << soSach << endl;
    cout << "Đang mượn: " << sachDangMuon << endl;
    cout << "Còn lại: " << (soSach - sachDangMuon) << endl;
    cout << "Tổng tiền: " << fixed << setprecision(0) << tienSach << " VNĐ\n";

    cout << BOLD << GREEN << "\n--- TẠP CHÍ ---\n" << RESET;
    cout << "Tổng số: " << soTapChi << endl;
    cout << "Đang mượn: " << tapChiDangMuon << endl;
    cout << "Còn lại: " << (soTapChi - tapChiDangMuon) << endl;
    cout << "Tổng tiền: " << fixed << setprecision(0) << tienTapChi << " VNĐ\n";

    cout << BOLD << GREEN << "\n--- BÁO ---\n" << RESET;
    cout << "Tổng số: " << soBao << endl;
    cout << "Đang mượn: " << baoDangMuon << endl;
    cout << "Còn lại: " << (soBao - baoDangMuon) << endl;
    cout << "Tổng tiền: " << fixed << setprecision(0) << tienBao << " VNĐ\n";

    cout << BOLD << RED << "\n=== TỔNG TOÀN BỘ ===\n" << RESET;
    cout << "Tổng tài liệu: " << tong << endl;
    cout << "Tổng đang mượn: " << tongDangMuon << endl;
    cout << "Tổng còn lại: " << (tong - tongDangMuon) << endl;
    cout << "Tổng tiền tất cả: " << fixed << setprecision(0) << tongTien
         << " VNĐ\n";
  }

  // Hàm timKiemTaiLieu: Hỗ trợ tìm kiếm nhanh tài liệu theo Mã hoặc 1 phần Tên
  // (không phân biệt viết hoa/thường).
  void timKiemTaiLieu(const vector<TaiLieu *> &danhSach) {
    string keyword;
    cout << "Nhập mã hoặc tên tài liệu cần tìm: ";
    clearBuffer();
    getline(cin, keyword);

    // Ép toàn bộ từ khóa tìm kiếm sang chữ viết thường để chuẩn hóa độ chính
    // xác
    string keywordToLower = toLowerCase(keyword);

    bool found = false;
    for (const auto &tl : danhSach) {
      if (tl != nullptr) {
        string maToLower = toLowerCase(tl->getMaTaiLieu());
        string tenToLower = toLowerCase(tl->getTenTaiLieu());

        // Kích hoạt ghép khớp nếu từ khóa trùng tuyệt đối mã, HOẶC nằm xen giữa
        // tên tài liệu
        if (maToLower == keywordToLower ||
            tenToLower.find(keywordToLower) != string::npos) {
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

  // Hàm muonTaiLieu: Trích chuyển trạng thái tài liệu sang 'Đang mượn' và khóa
  // lại bằng tên người mượn.
  void muonTaiLieu(vector<TaiLieu *> &danhSach, User *currentUser) {
    if (currentUser->getRole() != "Reader") {
      cout << RED << "Chỉ Reader mới có quyền mượn tài liệu!" << RESET << "\n";
      return;
    }
    string ma;
    cout << "Nhập mã tài liệu muốn mượn: ";
    cin >> ma;
    clearBuffer();

    string maToLower = toLowerCase(ma);

    for (auto &tl : danhSach) {
      if (tl != nullptr && toLowerCase(tl->getMaTaiLieu()) == maToLower) {
        if (tl->getIsBorrowed()) {
          cout << RED << "Tài liệu này đã được mượn bởi "
               << tl->getBorrowerName() << "!" << RESET << "\n";
        } else {
          // Khóa cờ đánh dấu là đã bị mang đi
          tl->setIsBorrowed(true);
          // Gắn biên lai tên người dùng đang mượn trực tiếp vào tài liệu
          tl->setBorrowerName(currentUser->getUsername());
          LuuDuLieu(danhSach);
          cout << GREEN << "Mượn tài liệu thành công!" << RESET << "\n";
        }
        return;
      }
    }
    cout << RED << "Không tìm thấy mã tài liệu!" << RESET << "\n";
  }

  // Hàm traTaiLieu: Đổi trạng thái tái lưu trữ thành sẵn sàng và gỡ bỏ tên
  // người mượn.
  void traTaiLieu(vector<TaiLieu *> &danhSach, User *currentUser) {
    if (currentUser->getRole() != "Reader") {
      cout << RED << "Chỉ Reader mới có quyền trả tài liệu!" << RESET << "\n";
      return;
    }
    string ma;
    cout << "Nhập mã tài liệu muốn trả: ";
    cin >> ma;
    clearBuffer();

    string maToLower = toLowerCase(ma);

    for (auto &tl : danhSach) {
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
          LuuDuLieu(danhSach);
          cout << GREEN << "Trả tài liệu thành công!" << RESET << "\n";
        }
        return;
      }
    }
    cout << RED << "Không tìm thấy mã tài liệu!" << RESET << "\n";
  }

  // Hàm xoaTaiLieu: Giải phóng hoàn toàn RAM (heap) của tài liệu và đá văng
  // khỏi danh sách (chặn xóa nếu đang có người mượn).
  void xoaTaiLieu(vector<TaiLieu *> &danhSach) {
    if (danhSach.empty()) {
      cout << RED << "Danh sách trống" << RESET << endl;
      return;
    }
    string maCanXoa;
    cout << "Nhập mã tài liệu cần xóa: ";
    cin >> maCanXoa;
    clearBuffer();

    string maToLower = toLowerCase(maCanXoa);

    // Tạo một mảng phụ quét gom những vị trí có nhiều bản sao trùng mã với nhau
    vector<int> viTriXoa;
    for (int i = 0; i < (int)danhSach.size(); i++) {
      if (toLowerCase(danhSach[i]->getMaTaiLieu()) == maToLower) {
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
      danhSach[index]->HienThiThongTin(cout);
      cout << endl;
    } else {
      for (int i = 0; i < (int)viTriXoa.size(); i++) {
        cout << BOLD << YELLOW << "Lựa chọn số " << (i + 1) << ":" << RESET
             << endl;
        danhSach[viTriXoa[i]]->HienThiThongTin(cout);
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
      danhSach[index]->HienThiThongTin(cout);
      cout << endl;
    }
    cout << string(155, '-') << endl;

    if (danhSach[index]->getIsBorrowed()) {
      cout << BOLD << RED << "LỖI: Trạng thái hiện tại là 'Đang mượn'." << endl;
      cout << "Tài liệu đang được mượn bởi "
           << danhSach[index]->getBorrowerName() << ". Không thể xóa!" << RESET
           << endl;
      return;
    }

    char xacNhan;
    cout << BOLD << RED
         << "Bạn có chắc chắn muốn xóa tài liệu này? (Y/N): " << RESET;
    cin >> xacNhan;
    clearBuffer();
    if (xacNhan == 'Y' || xacNhan == 'y') {
      // Lệnh xóa: Phá hủy thu hồi hoàn toàn khoảng nhớ RAM tĩnh
      delete danhSach[index];
      // Cắt đứt mắt xích chứa tài liệu này khỏi bộ khung danh sách chính
      danhSach.erase(danhSach.begin() + index);
      LuuDuLieu(danhSach);
      cout << GREEN << "Xóa tài liệu thành công!" << RESET << endl;
    } else {
      cout << YELLOW << "Hủy xóa tài liệu." << RESET << endl;
    }
  }
};

// Giải thích: Nơi chương trình bắt đầu chạy, chứa vòng lặp hiển thị Menu Tương
// tác cho người dùng.
int main() {
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);

  // Kích hoạt Lõi Hệ thống Thư viện tổng (Gom toàn thủ tục vận hành và RAM vào
  // đây)
  LibrarySystem library;
  library.LoadDuLieu(library.getDS());

  while (true) {
    // Rào chắn bảo mật: Buộc phải xác thực Account đi qua mới được vào lõi giao
    // tiếp
    User *currentUser = AuthManager::login();
    if (currentUser == nullptr) {
      cout << "Đã hủy đăng nhập.\n";
      break;
    }


    // Khai báo biến lưu giữ phím số người dùng vừa bấm để gọi Trình đơn (Menu)
    int luaChon;

    do {
      cout << "\n"
           << BOLD << GREEN << "=== HỆ THỐNG QUẢN LÝ THƯ VIỆN ===" << RESET
           << "\n";
      cout << CYAN << "Xin chào, " << currentUser->getUsername() << " ("
           << currentUser->getRole() << ")\n"
           << RESET;

      // Tùy theo chức vụ mà khởi động luồng logic xử lý case (trường hợp) tương
      // ứng
      if (currentUser->getRole() == "Librarian") {
        cout << "1. Thêm Sách\n";
        cout << "2. Thêm Tạp Chí\n";
        cout << "3. Thêm Báo\n";
        cout << "4. Hiển thị danh sách và Tính tiền\n";
        cout << "5. Thống kê chi tiết\n";
        cout << "6. Xóa tài liệu\n";
        cout << "0. Thoát và Đăng xuất\n";
      } else { // Reader menu
        cout << "1. Tìm kiếm tài liệu\n";
        cout << "2. Mượn tài liệu\n";
        cout << "3. Trả tài liệu\n";
        cout << "4. Xem danh sách tài liệu\n";
        cout << "0. Thoát và Đăng xuất\n";
      }

      cout << "Nhập lựa chọn của bạn: ";
      if (!(cin >> luaChon)) {
        cout << RED << "Lỗi: Lựa chọn không hợp lệ!\n" << RESET;
        clearBuffer();
        luaChon = -1;
        continue;
      }

      // Tùy theo chức vụ mà khởi động luồng logic xử lý case (trường hợp) tương
      // ứng
      if (currentUser->getRole() == "Librarian") {
        TaiLieu *tlMoi = nullptr;

        switch (luaChon) {
        case 1:
          cout << CYAN << "\nNhập thông tin Sách" << RESET << endl;
          // Tạo vỏ bọc mới cấp phát trên RAM dọn chỗ chuẩn bị chứa Sách
          tlMoi = new Sach();
          break;
        case 2:
          cout << CYAN << "\nNhập thông tin Tạp chí" << RESET << endl;
          tlMoi = new TapChi();
          break;
        case 3:
          cout << CYAN << "\nNhập thông tin Báo" << RESET << endl;
          tlMoi = new Bao();
          break;
        case 4:
          if (library.getDS().empty()) {
            cout << RED << "Danh sách trống!" << RESET << endl;
          } else {
            cout << "\n"
                 << BOLD << GREEN << "DANH SÁCH TÀI LIỆU" << RESET << "\n";
            InTieuDeBang();
            double tongTien = 0;
            for (const auto &tl : library.getDS()) {
              tl->HienThiThongTin(cout);
              tongTien += tl->TinhTien();
            }
            cout << string(155, '-') << endl;
            cout << BOLD << RED << "TỔNG TIỀN TẤT CẢ TÀI LIỆU: " << fixed
                 << setprecision(0) << tongTien << " VNĐ" << RESET << endl;
          }
          break;
        case 5:
          library.thongKeChiTiet(library.getDS());
          break;
        case 6:
          library.xoaTaiLieu(library.getDS());
          break;
        case 0:
          cout << "Đang đăng xuất...\n";
          break;
        default:
          cout << RED << "Lựa chọn không hợp lệ. Vui lòng nhập lại!\n" << RESET;
          clearBuffer();
        }

        if (tlMoi != nullptr) {
          tlMoi->Nhap(cin, library.getDS());
          // Đẩy nhồi dữ liệu vỏ bọc vừa rồi ghép nối vào toa tàu cấp cao (Danh
          // sách chung)
          library.getDS().push_back(tlMoi);
          cout << GREEN << "Thêm tài liệu thành công!\n" << RESET;
          library.LuuDuLieu(library.getDS());
        }
      } else { // Handle Reader actions
        switch (luaChon) {
        case 1:
          library.timKiemTaiLieu(library.getDS());
          break;
        case 2:
          library.muonTaiLieu(library.getDS(), currentUser);
          break;
        case 3:
          library.traTaiLieu(library.getDS(), currentUser);
          break;
        case 4:
          if (library.getDS().empty()) {
            cout << RED << "Danh sách trống!" << RESET << endl;
          } else {
            cout << "\n"
                 << BOLD << GREEN << "DANH SÁCH TÀI LIỆU" << RESET << "\n";
            InTieuDeBang();
            for (const auto &tl : library.getDS()) {
              tl->HienThiThongTin(cout);
            }
            cout << string(155, '-') << endl;
          }
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

    // Giải phóng thông tin ca gác cũ lúc bị ấn 0 thoát hệ thống
    delete currentUser;
    currentUser = nullptr;
  }

  return 0;
}
