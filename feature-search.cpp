// Nạp các thư viện chuẩn cần thiết cho chương trình
#include <fstream> // Thư viện dùng để đọc/ghi file dữ liệu
#include <iomanip> // Cung cấp các thao tác định dạng căn trích như setw (cột) và left (căn trái)
#include <iostream> // Cung cấp các luồng nhập/xuất chuẩn như cin, cout
#include <limits> // Cung cấp giới hạn kiểu số, dùng hữu ích khi kết hợp thao tác xóa bộ nhớ đệm
#include <sstream> // Thư viện dùng để xử lý cắt chuỗi (parsing) dữ liệu từ file
#include <string>  // Cung cấp kiểu dữ liệu chuỗi kí tự dạng std::string
#include <vector> // Cung cấp cấu trúc mảng động std::vector nhằm quản lý danh sách tài liệu linh hoạt
#include <algorithm> // Thư viện chứa std::transform
#include <cctype> // Thư viện chứa ::tolower
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

// ==========================================================
// 0. LỚP NGƯỜI DÙNG: User vả Xác Thực (Authentication)
// ==========================================================

class User {
private:
    string username;
    string password;
    string role;

public:
    User(string uname, string pwd, string r)
        : username(uname), password(pwd), role(r) {
    }
    virtual ~User() {}

    string getUsername() const { return username; }
    string getPassword() const { return password; }
    string getRole() const { return role; }
};

class Librarian : public User {
public:
    Librarian(string uname, string pwd) : User(uname, pwd, "Librarian") {}
};

class Reader : public User {
public:
    Reader(string uname, string pwd) : User(uname, pwd, "Reader") {}
};

class AuthManager {
public:
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

    static User* login() {
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
            bool found = false;
            while (getline(ifs, line)) {
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
        isBorrowed(false), borrowerName("") {
    }

    // Hàm Constructor có nhận các tham số dùng để khởi gán đối tượng lập tức
    TaiLieu(string ma, string ten, string nxb, int nam)
        : maTaiLieu(ma), tenTaiLieu(ten), nhaXuatBan(nxb), namXuatBan(nam),
        isBorrowed(false), borrowerName("") {
    }

    // Hàm Destructor ảo đảm bảo máy tính xóa đúng bộ nhớ rác khi đối tượng dẫn
    // xuất kết thúc
    virtual ~TaiLieu() {}

    // Tập hợp Getter / Setter theo giao diện điều khiển (interface) an toàn
    // lấy/gán dữ liệu (2.0đ)
    string getMaTaiLieu() const { return maTaiLieu; }
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
    virtual void Nhap(istream& is, const vector<TaiLieu*>& danhSach) = 0;

    // Khai báo hàm ảo hiển thị chuỗi nội dung với việc đẩy cột thẳng hàng
    virtual void HienThiThongTin(ostream& os) const = 0;

    // Phương thức thuần ảo ép lớp nhận kế thừa bắt buộc phải viết lại
    // Hàm này chặn người dùng khởi tạo trực tiếp instance ảo Tài Liệu không xác
    // định
    virtual double TinhTien() const = 0;

    // Phương thức ảo để lưu dữ liệu vào stream (phục vụ lưu file)
    virtual void Luu(ostream& os) const = 0;
};

// Cung cấp định nghĩa cho các hàm thuần ảo để các lớp con có thể tái sử dụng
// logic nhập gốc
inline void TaiLieu::Nhap(istream& is, const vector<TaiLieu*>& danhSach) {
    // Giải thuật kiểm tra mã tài liệu duy nhất
    while (true) {
        cout << "Nhập mã tài liệu: ";
        is >> maTaiLieu;
        bool biTrùng = false;
        for (const auto& tl : danhSach) {
            if (tl != nullptr && tl->getMaTaiLieu() == maTaiLieu) {
                biTrùng = true;
                break;
            }
        }
        if (biTrùng) {
            cout << RED << "Lỗi: Mã tài liệu '" << maTaiLieu
                << "' đã tồn tại! Vui lòng nhập mã khác." << RESET << endl;
        }
        else {
            break;
        }
    }

    clearBuffer(); // Phải dọn bộ nhớ ngay sau đó để chuẩn bị lấy getline có đoạn
    // trống

    cout << "Nhập tên tài liệu: ";
    getline(is, tenTaiLieu); // Đọc trọn cả dòng kể cả khoảng trắng xen kẽ

    cout << "Nhập nhà xuất bản: ";
    getline(is, nhaXuatBan);

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
inline void TaiLieu::HienThiThongTin(ostream& os) const {
    // Ép in về phía trái của cột, với cỡ chiều rộng setw cố định cho từng loại
    // thông tin
    os << left << setw(10) << maTaiLieu << setw(25) << tenTaiLieu << setw(20)
        << nhaXuatBan << setw(10) << namXuatBan;

    // Định dạng lại cột "Tình Trạng" (25 visual columns) bằng thuật toán bù byte
    if (isBorrowed) {
        os << setw(15 + sizeof("Mượn bởi: ") - 1) << ("Mượn bởi: " + borrowerName);
    }
    else {
        os << setw(17 + sizeof("Sẵn sàng") - 1) << "Sẵn sàng";
    }
}

// Định nghĩa phần lưu cơ sở cho các thuộc tính chung
inline void TaiLieu::Luu(ostream& os) const {
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
        : TaiLieu(ma, ten, nxb, nam), soTrang(st), tacGia(tg) {
    }

    // Lấy hoặc gán lượng trang của loại Sách
    int getSoTrang() const { return soTrang; }
    void setSoTrang(int st) { soTrang = st; }

    // Lấy hoặc gán đặc tả tác giả của đối tượng Sách
    string getTacGia() const { return tacGia; }
    void setTacGia(string tg) { tacGia = tg; }

    // Ghi đè hàm Nhập() của Lớp TaiLieu để lấy thêm Tác giả và Số trang
    void Nhap(istream& is, const vector<TaiLieu*>& danhSach) override {
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
    }

    // Ghi đè phương thức Hiện thị (Tùy chỉnh khoảng cột với phép bù sizeof để
    // UTF-8 hoạt động chuẩn)
    void HienThiThongTin(ostream& os) const override {
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
    void Luu(ostream& os) const override {
        os << "1|";
        TaiLieu::Luu(os);
        os << "|" << soTrang << "|" << tacGia << endl;
    }

    // Toán tử quá tải nhập/xuất riêng cho lớp Sách (khớp với sơ đồ UML)
    friend istream& operator>>(istream& is, Sach& s) {
        // Lưu ý: operator>> không có truy cập vào danhSach để check duy nhất,
        // nên khuyến khích dùng Nhap(is, danhSach) trực tiếp.
        s.Nhap(is, vector<TaiLieu*>());
        return is;
    }

    friend ostream& operator<<(ostream& os, Sach& s) {
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
        : TaiLieu(ma, ten, nxb, nam), soPhatHanh(sph), thangPhatHanh(tph) {
    }

    int getSoPhatHanh() const { return soPhatHanh; }
    void setSoPhatHanh(int sph) { soPhatHanh = sph; }

    int getThangPhatHanh() const { return thangPhatHanh; }
    void setThangPhatHanh(int tph) { thangPhatHanh = tph; }

    // Kịch bản Nhập thêm đối với loại Tạp Chí yêu cầu rà soát giá trị tháng (từ
    // 1-12)
    void Nhap(istream& is, const vector<TaiLieu*>& danhSach) override {
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
    void HienThiThongTin(ostream& os) const override {
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
    void Luu(ostream& os) const override {
        os << "2|";
        TaiLieu::Luu(os);
        os << "|" << soPhatHanh << "|" << thangPhatHanh << endl;
    }

    // Toán tử quá tải nhập/xuất riêng cho lớp Tạp chí (khớp với sơ đồ UML)
    friend istream& operator>>(istream& is, TapChi& tc) {
        tc.Nhap(is, vector<TaiLieu*>());
        return is;
    }

    friend ostream& operator<<(ostream& os, TapChi& tc) {
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
        : TaiLieu(ma, ten, nxb, nam), ngayPhatHanh(nph) {
    }

    int getNgayPhatHanh() const { return ngayPhatHanh; }
    void setNgayPhatHanh(int nph) { ngayPhatHanh = nph; }

    // Xông chức năng Nhập của Báo đảm bảo thu hẹp mốc đầu vào của ngày trong
    // thang 1..31
    void Nhap(istream& is, const vector<TaiLieu*>& danhSach) override {
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
    void HienThiThongTin(ostream& os) const override {
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
    void Luu(ostream& os) const override {
        os << "3|";
        TaiLieu::Luu(os);
        os << "|" << ngayPhatHanh << endl;
    }

    // Toán tử quá tải nhập/xuất riêng cho lớp Báo (khớp với sơ đồ UML)
    friend istream& operator>>(istream& is, Bao& b) {
        b.Nhap(is, vector<TaiLieu*>());
        return is;
    }

    friend ostream& operator<<(ostream& os, Bao& b) {
        b.HienThiThongTin(os);
        return os;
    }
};

// ==========================================================
// 3. CHƯƠNG TRÌNH CHÍNH (Vận hành trung tâm Console)
// ==========================================================

// Hàm vẽ khung đỉnh và đề mục bảng hiển thị danh sách chi tiết (xử lý byte bù
// trừ cho UTF-8 an toàn)
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

// Hàm thực hiện ghi toàn bộ danh sách hiện có vào file data.txt
void LuuDuLieu(const vector<TaiLieu*>& ds) {
    ofstream ofs("data.txt");
    if (ofs.is_open()) {
        for (const auto& tl : ds) {
            if (tl != nullptr) {
                tl->Luu(ofs);
            }
        }
        ofs.close();
    }
}

// Hàm thực hiện nạp dữ liệu từ file khi khởi động chương trình
void LoadDuLieu(vector<TaiLieu*>& ds) {
    ifstream ifs("data.txt");
    if (!ifs.is_open())
        return; // Nếu chưa có file thì bỏ qua

    string line;
    while (getline(ifs, line)) {
        if (line.empty())
            continue;
        stringstream ss(line);
        string typeStr, ma, ten, nxb, namStr, borrowStr, borrower;

        // Cắt chuỗi dựa trên ký tự gạch đứng |
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

            TaiLieu* newItem = nullptr;

            if (type == 1) { // Sách
                string stStr, tacGia;
                getline(ss, stStr, '|');
                getline(ss, tacGia, '|');
                newItem = new Sach(ma, ten, nxb, stoi(namStr), stoi(stStr), tacGia);
            }
            else if (type == 2) { // Tạp chí
                string sphStr, tphStr;
                getline(ss, sphStr, '|');
                getline(ss, tphStr, '|');
                newItem =
                    new TapChi(ma, ten, nxb, stoi(namStr), stoi(sphStr), stoi(tphStr));
            }
            else if (type == 3) { // Báo
                string nphStr;
                getline(ss, nphStr, '|');
                newItem = new Bao(ma, ten, nxb, stoi(namStr), stoi(nphStr));
            }

            if (newItem != nullptr) {
                newItem->setIsBorrowed(borrowStr == "1");
                newItem->setBorrowerName(borrower);
                ds.push_back(newItem);
            }
        }
        catch (...) {
            // Bỏ qua dòng bị lỗi định dạng số liệu từ data.txt
            continue;
        }
    }
    ifs.close();
}

// ==== HÀM NGHIỆP VỤ (Borrowing Logic) ====

// Hàm tiện ích chuyển sang chữ thường (Case-insensitive)
string toLowerCase(string s) {
    transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return tolower(c); });
    return s;
}

// Hàm tìm kiếm tài liệu theo MÃ (Search by ID)
void timKiemTheoMa(const vector<TaiLieu*>& ds) {

    string maCanTim;  // Biến lưu mã tài liệu người dùng nhập

    cout << "Nhập mã tài liệu cần tìm: ";

    cin.ignore();
    getline(cin, maCanTim);
    // Nhập cả dòng (có thể chứa khoảng trắng nếu cần)

    bool timThay = false;
    // kiểm tra có tìm thấy tài liệu hay không

    for (const auto& tl : ds) {
        // Duyệt từng tài liệu trong danh sách

        // So sánh CHÍNH XÁC:
        // - Phân biệt hoa thường (S01 khác s01)
        // - Không cho phép tìm một phần (phải nhập đúng 100%)
        if (tl != nullptr && tl->getMaTaiLieu() == maCanTim) {

            if (!timThay) {
                // Nếu đây là kết quả đầu tiên tìm được

                cout << "\n"
                    << BOLD << GREEN << "KẾT QUẢ TÌM KIẾM THEO MÃ" << RESET << "\n";

                InTieuDeBang(); // In tiêu đề bảng
                timThay = true; // Đánh dấu đã tìm thấy
            }

            tl->HienThiThongTin(cout);
            // Gọi hàm đa hình để hiển thị thông tin tài liệu
        }
    }

    if (!timThay) {
        // Nếu không tìm thấy tài liệu nào
        cout << RED
            << "Không tìm thấy tài liệu có mã: " << maCanTim
            << RESET << endl;
    }
    else {
        // Nếu có kết quả thì in dòng kẻ cuối bảng
        cout << string(155, '-') << endl;
    }
}

// Hàm hỗ trợ Reader: Mượn tài liệu
void muonTaiLieu(vector<TaiLieu*>& ds, User* currentUser) {
    if (currentUser->getRole() != "Reader") {
        cout << RED << "Chỉ Reader mới có quyền mượn tài liệu!" << RESET << "\n";
        return;
    }
    string ma;
    cout << "Nhập mã tài liệu muốn mượn: ";
    cin >> ma;

    for (auto& tl : ds) {
        if (tl != nullptr && tl->getMaTaiLieu() == ma) {
            if (tl->getIsBorrowed()) {
                cout << RED << "Tài liệu này đã được mượn bởi " << tl->getBorrowerName()
                    << "!" << RESET << "\n";
            }
            else {
                tl->setIsBorrowed(true);
                tl->setBorrowerName(currentUser->getUsername());
                LuuDuLieu(ds); // Lưu trạng thái
                cout << GREEN << "Mượn tài liệu thành công!" << RESET << "\n";
            }
            return;
        }
    }
    cout << RED << "Không tìm thấy mã tài liệu!" << RESET << "\n";
}

// Hàm hỗ trợ Reader: Trả tài liệu
void traTaiLieu(vector<TaiLieu*>& ds, User* currentUser) {
    if (currentUser->getRole() != "Reader") {
        cout << RED << "Chỉ Reader mới có quyền trả tài liệu!" << RESET << "\n";
        return;
    }
    string ma;
    cout << "Nhập mã tài liệu muốn trả: ";
    cin >> ma;

    for (auto& tl : ds) {
        if (tl != nullptr && tl->getMaTaiLieu() == ma) {
            if (!tl->getIsBorrowed()) {
                cout << YELLOW << "Tài liệu này đang rảnh (chưa ai mượn)." << RESET
                    << "\n";
            }
            else if (tl->getBorrowerName() != currentUser->getUsername()) {
                cout << RED << "Bạn không thể trả tài liệu do người khác ("
                    << tl->getBorrowerName() << ") mượn!" << RESET << "\n";
            }
            else {
                tl->setIsBorrowed(false);
                tl->setBorrowerName("");
                LuuDuLieu(ds); // Lưu trạng thái
                cout << GREEN << "Trả tài liệu thành công!" << RESET << "\n";
            }
            return;
        }
    }
    cout << RED << "Không tìm thấy mã tài liệu!" << RESET << "\n";
}

int main() {
    // Lệnh can thiệp chèn cấu trúc mã chữ hệ thống UTF-8 giúp Windows chạy được
    // ký tự Tiếng Việt (có dấu)
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    // Khởi tạo một mảng động (Vector) dùng con trỏ tĩnh làm mẫu chốt (nhảy bước
    // thiết lập mảng Đa Hình)
    vector<TaiLieu*> danhSachTaiLieu;

    // Tự động nạp dữ liệu từ file đã lưu
    LoadDuLieu(danhSachTaiLieu);

    User* currentUser = AuthManager::login();
    if (currentUser == nullptr) {
        cout << "Đã hủy đăng nhập.\n";
        return 0;
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
            cout << "1. Thêm Sách\n";
            cout << "2. Thêm Tạp Chí\n";
            cout << "3. Thêm Báo\n";
            cout << "4. Hiển thị danh sách và Tính tiền\n";
            cout << "5. Tìm kiếm theo mã\n";
            cout << "0. Thoát và Đăng xuất\n";

        }

        else { // Reader menu
            cout << "1. Tìm kiếm tài liệu\n";
            cout << "2. Mượn tài liệu\n";
            cout << "3. Trả tài liệu\n";
            cout << "4. Xem danh sách tài liệu\n";
            cout << "5. Tìm kiếm theo mã\n";
            cout << "0. Thoát và Đăng xuất\n";
        }

        cout << "Nhập lựa chọn của bạn: ";
        if (!(cin >> luaChon)) {
            cout << RED << "Lỗi: Lựa chọn không hợp lệ!\n" << RESET;
            clearBuffer();
            luaChon = -1;
            continue;
        }

        if (currentUser->getRole() == "Librarian") {
            TaiLieu* tlMoi = nullptr;

            switch (luaChon) {
            case 1:
                cout << CYAN << "\nNhập thông tin Sách" << RESET << endl;
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
                if (danhSachTaiLieu.empty()) {
                    cout << RED << "Danh sách trống!" << RESET << endl;
                }
                else {
                    cout << "\n"
                        << BOLD << GREEN << "DANH SÁCH TÀI LIỆU" << RESET << "\n";
                    InTieuDeBang();
                    double tongTien = 0;
                    for (const auto& tl : danhSachTaiLieu) {
                        tl->HienThiThongTin(cout);
                        tongTien += tl->TinhTien();
                    }
                    cout << string(155, '-') << endl;
                    cout << BOLD << RED << "TỔNG TIỀN TẤT CẢ TÀI LIỆU: " << fixed
                        << setprecision(0) << tongTien << " VNĐ" << RESET << endl;
                }
                break;
            case 5:
                timKiemTheoMa(danhSachTaiLieu); // Gọi hàm tìm kiếm theo mã
                break;
            case 0:
                cout << "Đang đăng xuất...\n";
                break;
            default:
                cout << RED << "Lựa chọn không hợp lệ. Vui lòng nhập lại!\n" << RESET;
                clearBuffer();
            }

            if (tlMoi != nullptr) {
                tlMoi->Nhap(cin, danhSachTaiLieu);
                danhSachTaiLieu.push_back(tlMoi);
                cout << GREEN << "Thêm tài liệu thành công!\n" << RESET;
                LuuDuLieu(danhSachTaiLieu);
            }
        }
        else { // Handle Reader actions
            switch (luaChon) {
            case 1:
                timKiemTheoMa(danhSachTaiLieu);
                break;
            case 2:
                muonTaiLieu(danhSachTaiLieu, currentUser);
                break;
            case 3:
                traTaiLieu(danhSachTaiLieu, currentUser);
                break;
            case 4:
                if (danhSachTaiLieu.empty()) {
                    cout << RED << "Danh sách trống!" << RESET << endl;
                }
                else {
                    cout << "\n"
                        << BOLD << GREEN << "DANH SÁCH TÀI LIỆU" << RESET << "\n";
                    InTieuDeBang();
                    for (const auto& tl : danhSachTaiLieu) {
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

    delete currentUser;
    currentUser = nullptr;
    // Xóa bỏ phế thải và thu hồi lại vùng heap memory mà C++ mượn làm lớp qua
    // Vector trước khi tắt máy
    for (auto tl : danhSachTaiLieu) {
        delete tl; // Khóa gỡ hoàn toàn trỏ đang neo vào heap (khởi động gọi
        // Destructor)
        tl = nullptr;
    }
    danhSachTaiLieu.clear(); // Gỡ toàn mặt xích khỏi hàng list để an toàn tuyệt
    // đối rò rỉ RAM

    return 0; // Báo hiệu mã biên dịch Main trả về cho điều hành OS là thành công
}
