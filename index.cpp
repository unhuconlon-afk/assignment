#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <vector>


using namespace std;

// === ĐỊNH NGHĨA MÃ MÀU ANSI ĐỂ LÀM ĐẸP CONSOLE (2.0đ) ===
#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define CYAN "\033[36m"
#define BOLD "\033[1m"

// Hàm tiện ích để xóa bộ nhớ đệm
void clearBuffer() {
  cin.clear();
  cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

// ==========================================================
// 1. LỚP CƠ SỞ: TaiLieu (Abstract Class)
// ==========================================================
class TaiLieu {
private:
  string maTaiLieu;
  string tenTaiLieu;
  string nhaXuatBan;
  int namXuatBan;

public:
  // Hàm khởi tạo (Constructor)
  TaiLieu() : maTaiLieu(""), tenTaiLieu(""), nhaXuatBan(""), namXuatBan(0) {}
  TaiLieu(string ma, string ten, string nxb, int nam)
      : maTaiLieu(ma), tenTaiLieu(ten), nhaXuatBan(nxb), namXuatBan(nam) {}
  virtual ~TaiLieu() {}

  // Getter / Setter đầy đủ (2.0đ)
  string getMaTaiLieu() const { return maTaiLieu; }
  void setMaTaiLieu(string ma) { maTaiLieu = ma; }

  string getTenTaiLieu() const { return tenTaiLieu; }
  void setTenTaiLieu(string ten) { tenTaiLieu = ten; }

  string getNhaXuatBan() const { return nhaXuatBan; }
  void setNhaXuatBan(string nxb) { nhaXuatBan = nxb; }

  int getNamXuatBan() const { return namXuatBan; }
  void setNamXuatBan(int nam) { namXuatBan = nam; }

  // Phương thức ảo để Đa hình (3.0đ)
  virtual void Nhap(istream &is) {
    cout << "Nhập mã tài liệu: ";
    is >> maTaiLieu;
    clearBuffer();
    cout << "Nhập tên tài liệu: ";
    getline(is, tenTaiLieu);
    cout << "Nhập nhà xuất bản: ";
    getline(is, nhaXuatBan);
    cout << "Nhập năm xuất bản: ";
    is >> namXuatBan;
  }

  virtual void HienThiThongTin(ostream &os) const {
    os << left << setw(10) << maTaiLieu << setw(25) << tenTaiLieu << setw(20)
       << nhaXuatBan << setw(10) << namXuatBan;
  }

  // Phương thức thuần ảo: Bắt buộc lớp con phải định nghĩa công thức tính tiền
  virtual double TinhTien() const = 0;

  // Quá tải toán tử nhập xuất (2.0đ) kết hợp với Đa hình
  friend istream &operator>>(istream &is, TaiLieu &tl) {
    tl.Nhap(is);
    return is;
  }
  friend ostream &operator<<(ostream &os, const TaiLieu &tl) {
    tl.HienThiThongTin(os);
    return os;
  }
};

// ==========================================================
// 2. LỚP KẾ THỪA: Sach, TapChi, Bao (3.0đ)
// ==========================================================

class Sach : public TaiLieu {
private:
  int soTrang;
  string tacGia;

public:
  Sach() : TaiLieu(), soTrang(0), tacGia("") {}

  // Ghi đè phương thức Nhập
  void Nhap(istream &is) override {
    TaiLieu::Nhap(is);
    cout << "Nhập số trang: ";
    is >> soTrang;
    clearBuffer();
    cout << "Nhập tác giả: ";
    getline(is, tacGia);
  }

  // Ghi đè phương thức Hiện thị (Sử dụng iomanip để gióng cột thẳng hàng)
  void HienThiThongTin(ostream &os) const override {
    TaiLieu::HienThiThongTin(os);
    os << left << setw(15) << "Sách" << setw(20) << tacGia << setw(15)
       << (to_string(soTrang) + " trang") << YELLOW << TinhTien() << RESET
       << endl;
  }

  // Công thức tính tiền riêng: Sách = Số trang * 500 VNĐ
  double TinhTien() const override { return soTrang * 500.0; }
};

class TapChi : public TaiLieu {
private:
  int soPhatHanh;
  int thangPhatHanh;

public:
  TapChi() : TaiLieu(), soPhatHanh(0), thangPhatHanh(0) {}

  void Nhap(istream &is) override {
    TaiLieu::Nhap(is);
    cout << "Nhập số phát hành: ";
    is >> soPhatHanh;
    cout << "Nhập tháng phát hành (1-12): ";
    is >> thangPhatHanh;
  }

  void HienThiThongTin(ostream &os) const override {
    TaiLieu::HienThiThongTin(os);
    string thongTinRieng =
        "Số: " + to_string(soPhatHanh) + ", T" + to_string(thangPhatHanh);
    os << left << setw(15) << "Tạp chí" << setw(20) << "N/A" << setw(15)
       << thongTinRieng << YELLOW << TinhTien() << RESET << endl;
  }

  // Công thức tính tiền riêng: Tạp chí = Giá cố định 25000 VNĐ
  double TinhTien() const override { return 25000.0; }
};

class Bao : public TaiLieu {
private:
  int ngayPhatHanh;

public:
  Bao() : TaiLieu(), ngayPhatHanh(0) {}

  void Nhap(istream &is) override {
    TaiLieu::Nhap(is);
    cout << "Nhập ngày phát hành (1-31): ";
    is >> ngayPhatHanh;
  }

  void HienThiThongTin(ostream &os) const override {
    TaiLieu::HienThiThongTin(os);
    string thongTinRieng = "Ngày: " + to_string(ngayPhatHanh);
    os << left << setw(15) << "Báo" << setw(20) << "N/A" << setw(15)
       << thongTinRieng << YELLOW << TinhTien() << RESET << endl;
  }

  // Công thức tính tiền riêng: Báo = Giá cố định 5000 VNĐ
  double TinhTien() const override { return 5000.0; }
};

// ==========================================================
// 3. CHƯƠNG TRÌNH CHÍNH
// ==========================================================
void InTieuDeBang() {
  cout << string(125, '-') << endl;
  cout << BOLD << CYAN << left << setw(10) << "Mã TL" << setw(25)
       << "Tên Tài Liệu" << setw(20) << "Nhà Xuất Bản" << setw(10) << "Năm XB"
       << setw(15) << "Phân Loại" << setw(20) << "Tác Giả" << setw(15)
       << "Chi Tiết"
       << "Giá Tiền" << RESET << endl;
  cout << string(125, '-') << endl;
}

int main() {
  // Sử dụng Vector chứa con trỏ lớp cơ sở để thể hiện Đa hình
  vector<TaiLieu *> danhSachTaiLieu;
  int luaChon;

  do {
    cout << "\n"
         << BOLD << GREEN << "=== HỆ THỐNG QUẢN LÝ THƯ VIỆN ===" << RESET
         << "\n";
    cout << "1. Thêm Sách\n";
    cout << "2. Thêm Tạp Chí\n";
    cout << "3. Thêm Báo\n";
    cout << "4. Hiển thị danh sách và Tính tiền\n";
    cout << "0. Thoát\n";
    cout << "Nhập lựa chọn của bạn: ";
    cin >> luaChon;

    TaiLieu *tlMoi = nullptr;

    switch (luaChon) {
    case 1:
      cout << CYAN << "\n--- Nhập thông tin Sách ---" << RESET << endl;
      tlMoi = new Sach();
      break;
    case 2:
      cout << CYAN << "\n--- Nhập thông tin Tạp chí ---" << RESET << endl;
      tlMoi = new TapChi();
      break;
    case 3:
      cout << CYAN << "\n--- Nhập thông tin Báo ---" << RESET << endl;
      tlMoi = new Bao();
      break;
    case 4:
      if (danhSachTaiLieu.empty()) {
        cout << RED << "Danh sách trống!" << RESET << endl;
      } else {
        cout << "\n" << BOLD << GREEN << "DANH SÁCH TÀI LIỆU" << RESET << "\n";
        InTieuDeBang();
        double tongTien = 0;
        for (const auto &tl : danhSachTaiLieu) {
          // Gọi toán tử xuất (<<), tự động kích hoạt Đa hình
          cout << *tl;
          tongTien += tl->TinhTien();
        }
        cout << string(125, '-') << endl;
        cout << BOLD << RED << "TỔNG TIỀN TẤT CẢ TÀI LIỆU: " << fixed
             << setprecision(0) << tongTien << " VNĐ" << RESET << endl;
      }
      break;
    case 0:
      cout << "Đang thoát chương trình...\n";
      break;
    default:
      cout << RED << "Lựa chọn không hợp lệ. Vui lòng nhập lại!\n" << RESET;
      clearBuffer();
    }

    // Nếu tạo mới thành công, gọi toán tử nhập (>>) kích hoạt Đa hình
    if (tlMoi != nullptr) {
      cin >> *tlMoi;
      danhSachTaiLieu.push_back(tlMoi);
      cout << GREEN << "Thêm tài liệu thành công!\n" << RESET;
    }

  } while (luaChon != 0);

  // Giải phóng bộ nhớ (Quy tắc bắt buộc khi dùng con trỏ)
  for (auto tl : danhSachTaiLieu) {
    delete tl;
  }
  danhSachTaiLieu.clear();

  return 0;
}