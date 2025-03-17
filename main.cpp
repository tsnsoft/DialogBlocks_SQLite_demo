#include <wx/wx.h> // Подключаем wxWidgets
#include <wx/listctrl.h> // Подключаем список
#include <sqlite3.h> // Подключаем SQLite
#include <thread> // Подключаем потоки
#include "tsnsoft.xpm" // Подключаем иконку

// Определение события для передачи данных из потока в основной поток
wxDEFINE_EVENT(EVT_LOAD_DATA, wxThreadEvent);

// Определение главного окна приложения
class MyFrame : public wxFrame {
public:
	// Конструктор главного окна
	MyFrame() : wxFrame(nullptr, wxID_ANY, wxString::FromUTF8("База данных дистрибутивов Linux"), wxDefaultPosition, wxSize(630, 350), wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX)) {
		Centre(); // Центрируем окно на экране
		SetIcon(wxIcon(tsnsoft_xpm)); // Устанавливаем иконку

		// Создаем список с колонками для отображения данных
		listCtrl = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);
		listCtrl->AppendColumn(wxString::FromUTF8("ID"), wxLIST_FORMAT_LEFT, 50);
		listCtrl->AppendColumn(wxString::FromUTF8("Название дистрибутива"), wxLIST_FORMAT_LEFT, 200);
		listCtrl->AppendColumn(wxString::FromUTF8("Создатель"), wxLIST_FORMAT_LEFT, 250);
		listCtrl->AppendColumn(wxString::FromUTF8("Год создания"), wxLIST_FORMAT_LEFT, 120);

		// Привязываем обработчик события загрузки данных
		Bind(EVT_LOAD_DATA, &MyFrame::OnLoadData, this);

		InitDatabase(); // Инициализация базы данных
		LoadDataAsync(); // Запуск загрузки данных в отдельном потоке
	}

	// Деструктор: закрываем соединение с базой данных
	~MyFrame() {
		sqlite3_close(db);
	}

private:
	wxListCtrl* listCtrl; // Элемент управления списком
	sqlite3* db = nullptr; // Указатель на базу данных SQLite

	// Функция инициализации базы данных
	void InitDatabase() {
		sqlite3_open("distributions.db", &db); // Открываем (или создаем) базу данных

		// Создаем таблицу, если она не существует
		sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS distributions(id INTEGER PRIMARY KEY, title TEXT, author TEXT, year INTEGER);", nullptr, nullptr, nullptr);
		sqlite3_exec(db, "DELETE FROM distributions;", nullptr, nullptr, nullptr); // Очищаем данные перед вставкой

		// Структура для хранения информации о дистрибутивах
		struct Distribution {
			std::string title; // Название дистрибутива
			std::string author; // Создатель дистрибутива
			int year;
		};

		// Создаем список дистрибутивов для заполнения базы данных
		std::vector<Distribution> distributions = {
			{"Ubuntu", "Mark Shuttleworth", 2004},
			{"Debian", "Ian Murdock", 1993},
			{"Linux Mint", "Clement Lefebvre", 2006},
			{"Fedora", "Warren Togami", 2003},
			{"Arch Linux", "Judd Vinet", 2002},
			{"Red Hat Linux", "Marc Ewing", 1993},
			{"Red Hat Enterprise Linux", "Marc Ewing", 2002},
			{"SUSE Linux", "Roland Dyroff", 1994},
			{"Manjaro", "Philip Müller", 2011},
			{"elementary OS", "Daniel Foré", 2011},
			{"Kali Linux", "Mati Aharoni", 2013},
			{"Gentoo", "Daniel Robbins", 2000},
			{"Slackware", "Patrick Volkerding", 1993}
		};

		// Вставляем записи в базу данных
		int id = 1;
		for (const auto& distribution : distributions) {
			std::string sql = "INSERT INTO distributions(id, title, author, year) VALUES(" + std::to_string(id) + ", '" + distribution.title + "', '" + distribution.author + "', " + std::to_string(distribution.year) + ");";
			sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr);
			id++;
		}
	}

	// Функция для загрузки данных в отдельном потоке
	void LoadDataAsync() {
		std::thread([this]() { // Создаем новый поток
			wxThreadEvent event(EVT_LOAD_DATA); // Создаем событие для передачи данных
			std::vector<std::tuple<int, std::string, std::string, int >> data; // Вектор для хранения данных

			sqlite3_stmt* stmt; // Указатель на запрос
			sqlite3_prepare_v2(db, "SELECT * FROM distributions", -1, &stmt, nullptr); // Подготавливаем запрос

			// Читаем данные из базы
			while (sqlite3_step(stmt) == SQLITE_ROW) { // Пока есть строки
				data.emplace_back( // Добавляем данные в вектор
					sqlite3_column_int(stmt, 0), // ID
					reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)), // Название
					reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)), // Автор
					sqlite3_column_int(stmt, 3) // Год
				);
			}
			sqlite3_finalize(stmt); // Завершаем запрос

			event.SetPayload(data); // Устанавливаем данные для передачи
			wxQueueEvent(this, event.Clone()); // Отправляем данные в основной поток
			}).detach(); // Запускаем поток
	}

	// Обработчик события загрузки данных в GUI
	void OnLoadData(wxThreadEvent& event) {
		auto data = event.GetPayload<std::vector<std::tuple<int, std::string, std::string, int>>>(); // Получаем данные из события
		listCtrl->DeleteAllItems(); // Очищаем список

		for (const auto& entry : data) { // Перебираем данные
			int id; // ID дистрибутива
			std::string title, author; // Название и автор дистрибутива
			int year; // Год создания
			std::tie(id, title, author, year) = entry; // Явное использование std::tie

			long index = listCtrl->InsertItem(listCtrl->GetItemCount(), wxString::Format("%d", id)); // Вставляем ID
			listCtrl->SetItem(index, 1, wxString::FromUTF8(title.c_str())); // Вставляем название
			listCtrl->SetItem(index, 2, wxString::FromUTF8(author.c_str())); // Вставляем автора
			listCtrl->SetItem(index, 3, wxString::Format("%d", year)); // Вставляем год
		}
	}

};

// Определение основного приложения wxWidgets
class MyApp :
	public wxApp {
public:
	virtual bool OnInit() {
		setlocale(LC_ALL, ""); // Устанавливаем поддержку русских символов
		MyFrame* frame = new MyFrame(); // Создаем главное окно
		frame->Show(true); // Показываем окно
		return true;
	}
};

wxIMPLEMENT_APP(MyApp); // Точка входа в приложение wxWidgets
