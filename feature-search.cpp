// Nạp các thư viện chuẩn cần thiết cho chương trình
#include <iomanip> // Cung cấp các thao tác định dạng căn trích như setw (cột) và left (căn trái)
#include <iostream> // Cung cấp các luồng nhập/xuất chuẩn như cin, cout
#include <limits> // Cung cấp giới hạn kiểu số, dùng hữu ích khi kết hợp thao tác xóa bộ nhớ đệm
#include <string> // Cung cấp kiểu dữ liệu chuỗi kí tự dạng std::string
#include <vector> // Cung cấp cấu trúc mảng động std::vector nhằm quản lý danh sách tài liệu linh hoạt
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
void clearBuffer(istream& is = cin) {
    is.clear(); // Hủy cờ lỗi trên luồng nhập khi người dùng lỡ nhập sai kiểu dữ
    // liệu chữ vào số
    is.ignore(
        numeric_limits<streamsize>::max(),
        '\n'); // Loại bỏ tất cả ký tự tồn đọng bao gồm cả ký tự Enter ('\n')
}

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

public:
    // Hàm Constructor (Khởi tạo) không tham số thiết lập giá trị chuẩn an toàn
    // lúc rỗng
    TaiLieu() : maTaiLieu(""), tenTaiLieu(""), nhaXuatBan(""), namXuatBan(0) {}

    // Hàm Constructor có nhận các tham số dùng để khởi gán đối tượng lập tức
    TaiLieu(string ma, string ten, string nxb, int nam)
        : maTaiLieu(ma), tenTaiLieu(ten), nhaXuatBan(nxb), namXuatBan(nam) {
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

    // Phương thức Đa hình thuần ảo (pure virtual) để xử lý bước nhập từng dữ liệu gốc
    // riêng Có thể được ghi đè bởi các lớp Sách/Báo nhờ cơ chế liên kết trễ (Late
    // binding) - (3.0đ)
    virtual void Nhap(istream& is) = 0;

    // Khai báo hàm thuần ảo hiển thị chuỗi nội dung với việc đẩy cột thẳng hàng
    virtual void HienThiThongTin(ostream& os) const = 0;

    // Phương thức thuần ảo ép lớp nhận kế thừa bắt buộc phải viết lại
    // Hàm này chặn người dùng khởi tạo trực tiếp instance ảo Tài Liệu không xác
    // định
    virtual double TinhTien() const = 0;

    // Quá tải luồng lấy dữ liệu tạo một thao tác nhập gọn qua cin >>
    friend istream& operator>>(istream& is, TaiLieu& tl) {
        tl.Nhap(is); // Cầu nối lấy gọi hàm đa hình, sẽ móc nối hàm nhập chuẩn tuỳ
        // loại object
        return is;
    }

    // Quá tải luồng đổ dữ liệu tạo một thao tác xuất gọn qua cout <<
    friend ostream& operator<<(ostream& os, const TaiLieu& tl) {
        tl.HienThiThongTin(
            os); // Cầu nối xuất lấy đúng nội dung đính kèm các đặc điểm riêng rẽ
        return os;
    }
};

// Cài đặt phần thân chung cho các hàm thuần ảo của lớp TaiLieu
// Các lớp con vẫn có thể gọi hàm này thông qua phạm vi TaiLieu::
inline void TaiLieu::Nhap(istream& is) {
    cout << "Nhập mã tài liệu: ";
    is >> maTaiLieu; // Nhập dữ liệu mã với từ duy nhất không có khoảng cách
    clearBuffer(is); // Phải dọn bộ nhớ ngay sau đó để chuẩn bị lấy getline có
    // đoạn trống

    cout << "Nhập tên tài liệu: ";
    getline(is, tenTaiLieu); // Đọc trọn cả dòng kể cả khoảng trắng xen kẽ

    cout << "Nhập nhà xuất bản: ";
    getline(is, nhaXuatBan);

    cout << "Nhập năm xuất bản: ";
    while (!(is >> namXuatBan)) {
        cout << RED
            << "Lỗi: Năm xuất bản phải là số. Vui lòng nhập lại: " << RESET;
        clearBuffer(is);
    }
}

inline void TaiLieu::HienThiThongTin(ostream& os) const {
    // Ép in về phía trái của cột, với cỡ chiều rộng setw cố định cho từng
    // loại thông tin
    os << left << setw(10) << maTaiLieu << setw(25) << tenTaiLieu << setw(20)
        << nhaXuatBan << setw(10) << namXuatBan;
};

// ==========================================================
// 2. LỚP KẾ THỪA: Đối tượng Sách, Tạp chí, và Báo (Phần Đa Hình 3.0đ)
// ==========================================================

class Sach : public TaiLieu { // Khởi xướng Kế Thừa tính năng gốc từ lớp cha
    // lớp 'TaiLieu'
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
    void Nhap(istream& is) override {
        TaiLieu::Nhap(is); // Ra lệnh gọi phần Nhập của lớp cơ sở giải quyết 4
        // thông số nền đè trước
        cout << "Nhập số trang: ";
        while (!(is >> soTrang)) {
            cout << RED << "Lỗi: Phải nhập số. Vui lòng nhập lại: " << RESET;
            clearBuffer(is);
        }
        clearBuffer(is); // Đề phòng lệnh getline phía sau bị nhảy do cặn Enter
        cout << "Nhập tác giả: ";
        getline(is, tacGia); // Lấy tên tác giả (cho phép khoảng cách)
    }

    // Ghi đè phương thức Hiện thị (Tùy chỉnh khoảng cột với phép bù sizeof để
    // UTF-8 hoạt động chuẩn)
    void HienThiThongTin(ostream& os) const override {
        TaiLieu::HienThiThongTin(os); // In 4 trường mặc định gốc trước
        // Căn trái và đẩy các trường thêm: Phân Loại (Sách), Tác Giả, Số Trang,
        // và Tinh Tiền đi kèm định dạng màu
        os << left << setw(11 + sizeof("Sách") - 1) << "Sách" << setw(20) << tacGia
            << setw(15) << (to_string(soTrang) + " trang") << YELLOW << setw(15)
            << TinhTien() << RESET << endl;
    }

    // Cấu hình quy tắc riêng trả về phép tính Tiền dựa vào Số Trang
    double TinhTien() const override {
        return soTrang * 500.0;
    } // Đồng giá 500đ nhân với lượng trang
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
    void Nhap(istream& is) override {
        TaiLieu::Nhap(is); // Điền thông số chuẩn gốc cho Tạp Chí
        cout << "Nhập số phát hành: ";
        while (!(is >> soPhatHanh)) {
            cout << RED << "Lỗi: Phải nhập số. Vui lòng nhập lại: " << RESET;
            clearBuffer(is);
        }

        // Khởi tạo khoá cản ngoại lệ kiểm soát các trị số nhập hợp lệ theo tháng
        // (1 đến 12)
        do {
            if (is.fail()) { // Bắt lỗi gõ luồng (Vd: nhập chữ vào int) để tránh
                // vòng lặp chết
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
    void Nhap(istream& is) override {
        TaiLieu::Nhap(is); // Điền thông số chuẩn cho Báo

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
};

// ==========================================================
// 3. CHƯƠNG TRÌNH CHÍNH (Vận hành trung tâm Console)
// ==========================================================

// Hàm vẽ khung đỉnh và đề mục bảng hiển thị danh sách chi tiết (xử lý byte bù
// trừ cho UTF-8 an toàn)
void InTieuDeBang() {
    cout << string(130, '-')
        << endl; // Đổ ra một dải đường ngang biên giới trên độ dài 130 ký hiệu
    cout << BOLD << CYAN << left << setw(5 + sizeof("Mã TL") - 1) << "Mã TL"
        << setw(13 + sizeof("Tên Tài Liệu") - 1) << "Tên Tài Liệu"
        << setw(8 + sizeof("Nhà Xuất Bản") - 1) << "Nhà Xuất Bản"
        << setw(4 + sizeof("Năm XB") - 1) << "Năm XB"
        << setw(6 + sizeof("Phân Loại") - 1) << "Phân Loại"
        << setw(13 + sizeof("Tác Giả") - 1) << "Tác Giả"
        << setw(7 + sizeof("Chi Tiết") - 1) << "Chi Tiết"
        << setw(7 + sizeof("Giá Tiền") - 1) << "Giá Tiền" << RESET
        << endl; // Chèn mã kết thúc làm màu về mặc định
    cout << string(130, '-')
        << endl; // Đổ đường viền phân tách các hàng thông tin số liệu chính yếu
}

int main() {
    // Lệnh can thiệp chèn cấu trúc mã chữ hệ thống UTF-8 giúp Windows chạy được
    // ký tự Tiếng Việt (có dấu)
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    // Khởi tạo một mảng động (Vector) dùng con trỏ tĩnh làm mẫu chốt (nhảy bước
    // thiết lập mảng Đa Hình)
    vector<TaiLieu*> danhSachTaiLieu;
    int luaChon; // Biến chọn lệnh kêt nối Menu điều hướng

    // Thuật toán khối khởi động vòng Menu liên tục, lặp đến khi có hiệu lệnh
    // Kết thúc
    do {
        // Biểu diễn phần tiêu đề và điều hướng người dùng
        cout << "\n"
            << BOLD << GREEN << "=== HỆ THỐNG QUẢN LÝ THƯ VIỆN ===" << RESET
            << "\n";
        cout << "1. Thêm Sách\n";
        cout << "2. Thêm Tạp Chí\n";
        cout << "3. Thêm Báo\n";
        cout << "4. Hiển thị danh sách và Tính tiền\n";
        cout << "5. Tìm kiếm theo mã\n";
        cout << "0. Thoát\n";
        cout << "Nhập lựa chọn của bạn: ";
        if (!(cin >> luaChon)) {
            cin.clear();
            luaChon = -1; // Ép vào default bắt lỗi nhập chữ
        }

        TaiLieu* tlMoi = nullptr; // Chốt biến gốc con trỏ về rỗng giúp an toàn
        // khi chọn lệch phím điều hướng

// Khối điều rẽ hướng điền chọn bằng thẻ (1 ra Sách, 2 ra Tạp chí...)
        switch (luaChon) {
        case 1:
            cout << CYAN << "\nNhập thông tin Sách" << RESET << endl;
            tlMoi = new Sach(); // Gọi vùng cấp phát bộ nhớ mới làm thuộc tinh là
            // một đối tượng thuộc loại Sách
            break;
        case 2:
            cout << CYAN << "\nNhập thông tin Tạp chí" << RESET << endl;
            tlMoi = new TapChi(); // Cấp vùng nhớ thể hiện Tạp Chí
            break;
        case 3:
            cout << CYAN << "\nNhập thông tin Báo" << RESET << endl;
            tlMoi = new Bao(); // Cấp vùng thể hiện biên niên quyển Báo
            break;
        case 4:
            // Phát thông điệp cản đường người dùng trong trường mảng còn hoàn toàn
            // trống
            if (danhSachTaiLieu.empty()) {
                cout << RED << "Danh sách trống!" << RESET << endl;
            }
            else {
                // Tái lập hiển thị toàn bộ nội dung nếu vector đang nắm giữ dữ liệu
                // thành phần
                cout << "\n" << BOLD << GREEN << "DANH SÁCH TÀI LIỆU" << RESET << "\n";
                InTieuDeBang(); // Xuất cụm header phía trên

                double tongTien = 0; // Thẻ gom giá trị nhằm tính tổng cước ấn phẩm

                // Quét lấy tất cả thiết đặt bản lưu trong Mảng (Chạy đa hình dựa vào
                // Type thực chất)
                for (const auto& tl : danhSachTaiLieu) {
                    // Gọi thao tác đổ thẳng qua operator Toán tử (<<) đa hình để phân
                    // rã ra Sách/Báo tự động
                    cout << *tl;
                    tongTien +=
                        tl->TinhTien(); // Hút giá tiền của mảng để tổng hợp ngân quỹ
                }
                cout << string(130, '-') << endl; // Kẻ vạch đóng cấu tạo bảng ấn phẩm

                // Hiện thông báo tiền tệ kết toán với setprecision chặn không lộ dấu
                // chấm trôi nổi (phân số thập phân)
                cout << BOLD << RED << "TỔNG TIỀN TẤT CẢ TÀI LIỆU: " << fixed
                    << setprecision(0) << tongTien << " VNĐ" << RESET << endl;
            }
            break;
        case 5: {
            // Kiểm tra nếu danh sách tài liệu rỗng
            if (danhSachTaiLieu.empty()) {
                // In thông báo danh sách trống (màu đỏ)
                cout << RED << "Danh sách trống!" << RESET << endl;
                break; // Thoát khỏi case 5
            }

            string maCanTim; // Biến lưu mã tài liệu cần tìm

            // Yêu cầu người dùng nhập mã tài liệu
            cout << "Nhập mã tài liệu cần tìm: ";
            cin >> maCanTim;

            bool timThay = false; // Biến cờ để kiểm tra có tìm thấy hay không

            // Duyệt qua toàn bộ danh sách tài liệu
            for (const auto& tl : danhSachTaiLieu) {
                // So sánh mã tài liệu hiện tại với mã cần tìm
                if (tl->getMaTaiLieu() == maCanTim) {

                    // Nếu đây là lần đầu tiên tìm thấy
                    if (!timThay) {
                        // In tiêu đề kết quả (chỉ in 1 lần)
                        cout << "\n" << BOLD << GREEN << "KẾT QUẢ TÌM KIẾM" << RESET << "\n";

                        // Gọi hàm in tiêu đề bảng
                        InTieuDeBang();
                    }

                    // In thông tin tài liệu (dùng toán tử << đã overload)
                    cout << *tl;

                    // Đánh dấu đã tìm thấy
                    timThay = true;
                }
            }

            // Sau khi duyệt xong, nếu không tìm thấy tài liệu nào
            if (!timThay) {
                // In thông báo không tìm thấy (màu đỏ)
                cout << RED << "Không tìm thấy tài liệu có mã: "
                    << maCanTim << RESET << endl;
            }

            break; 
        }

        case 0:
            cout << "Đang thoát chương trình...\n"; // Điểm báo dừng vòng quay
            // console trước khi trả về
            // System
            break;
        default:
            // Cảnh báo mã phản hồi với thao tác gõ mã chữ/số ngoài phạm trù từ 0 -
            // 4
            cout << RED << "Lựa chọn không hợp lệ. Vui lòng nhập lại!\n" << RESET;
            clearBuffer(); // Phải phá bộ nhớ lỗi chống rác dồn vòng lặp vô tận
        }

        // Thủ diễn khép luồng cho Mảng đẩy đối tượng thêm vào khi rẽ nhánh new
        // Obj() không thất bại (NULL)
        if (tlMoi != nullptr) {
            cin >> *tlMoi; // Tiến hành lùi về phần (>> Nhập) tự nhận hệ số đa hình
            // theo Object Sáng lập
            danhSachTaiLieu.push_back(
                tlMoi); // Chuỗi hàm chèn con tem con trỏ vào đáy giỏ Vector
            cout << GREEN << "Thêm tài liệu thành công!\n" << RESET;
        }

    } while (luaChon != 0); // Neo khoá cản cho rẽ ngang ở Case 0 Exit

    // Xóa bỏ phế thải và thu hồi lại vùng heap memory mà C++ mượn làm lớp qua
    // Vector trước khi tắt máy
    for (auto tl : danhSachTaiLieu) {
        delete tl; // Khóa gỡ hoàn toàn trỏ đang neo vào heap (khởi động gọi
        // Destructor)
    }
    danhSachTaiLieu.clear(); // Gỡ toàn mặt xích khỏi hàng list để an toàn tuyệt
    // đối rò rỉ RAM

    return 0; // Báo hiệu mã biên dịch Main trả về cho điều hành OS là thành
    // công
}