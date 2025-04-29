#include <wx/wx.h> // Подключаем основную библиотеку wxWidgets для создания GUI
#include <wx/listctrl.h> // Подключаем компонент wxListCtrl для отображения данных в виде таблицы
#include <sqlite3.h> // Подключаем библиотеку SQLite для работы с базой данных
#include <thread> // Подключаем библиотеку для работы с потоками
#include "tsnsoft.xpm" // Подключаем файл с иконкой приложения

wxDEFINE_EVENT(EVT_LOAD_DATA, wxThreadEvent); // Определяем пользовательское событие для передачи данных из потока

struct Distribution { // Базовая структура с общими полями по дистрибутивам
	wxString title; // Поле для хранения названия дистрибутива
	wxString author; // Поле для хранения имени создателя дистрибутива
	int year; // Поле для хранения года создания дистрибутива
};

struct TableFields : public Distribution { // Структура для хранения данных из базы
	int id; // Поле для хранения идентификатора дистрибутива
	// Наследует поля title, author, year из Distribution
};

class MyFrame : public wxFrame { // Определяем класс главного окна, наследуясь от wxFrame
public:
	MyFrame() : wxFrame(nullptr, wxID_ANY, wxString(wxT("База данных дистрибутивов Linux")),
		wxDefaultPosition, wxSize(630, 350),
		wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX)) { // Конструктор главного окна
		Centre(); // Центрируем окно на экране
		SetIcon(wxIcon(tsnsoft_xpm)); // Устанавливаем иконку приложения из файла tsnsoft.xpm

		// Создаём компонент списка для отображения данных
		listCtrl = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT); // Создаём компонент списка в стиле таблицы
		listCtrl->AppendColumn(wxT("ID"), wxLIST_FORMAT_LEFT, 50); // Добавляем колонку для ID с шириной 50 пикселей
		listCtrl->AppendColumn(wxT("Название дистрибутива"), wxLIST_FORMAT_LEFT, 200); // Добавляем колонку для названия с шириной 200 пикселей
		listCtrl->AppendColumn(wxT("Создатель"), wxLIST_FORMAT_LEFT, 250); // Добавляем колонку для создателя с шириной 250 пикселей
		listCtrl->AppendColumn(wxT("Год создания"), wxLIST_FORMAT_LEFT, 120); // Добавляем колонку для года с шириной 120 пикселей

		Bind(EVT_LOAD_DATA, &MyFrame::OnLoadData, this); // Привязываем обработчик события загрузки данных

		InitDatabase(); // Вызываем метод инициализации базы данных
		LoadDataAsync(); // Запускаем асинхронную загрузку данных
	}

	~MyFrame() { // Деструктор главного окна
		if (db) { // Проверяем, открыт ли указатель на базу данных
			sqlite3_close(db); // Закрываем соединение с базой данных
		}
	}

private:
	wxListCtrl* listCtrl; // Указатель на компонент списка для отображения данных
	sqlite3* db = nullptr; // Указатель на объект базы данных SQLite, изначально пустой

	void InitDatabase() { // Метод для инициализации базы данных
		if (sqlite3_open("distributions.db", &db) != SQLITE_OK) { // Открываем или создаём файл базы данных
			wxMessageBox(wxT("Ошибка открытия базы данных"), wxT("Ошибка"), wxICON_ERROR); // Показываем сообщение об ошибке, если не удалось открыть базу
			return; // Прерываем выполнение метода при ошибке
		}

		const char* createTable = "CREATE TABLE IF NOT EXISTS distributions("
			"id INTEGER PRIMARY KEY, title TEXT, author TEXT, year INTEGER);"; // SQL-запрос для создания таблицы
		sqlite3_exec(db, createTable, nullptr, nullptr, nullptr); // Выполняем запрос на создание таблицы, если она не существует

		sqlite3_exec(db, "DELETE FROM distributions;", nullptr, nullptr, nullptr); // Очищаем таблицу перед вставкой новых данных

		const std::vector<Distribution> distributions = { // Создаём вектор с данными дистрибутивов для вставки
			{wxT("Ubuntu"), wxT("Mark Shuttleworth"), 2004}, // Данные для Ubuntu
			{wxT("Debian"), wxT("Ian Murdock"), 1993}, // Данные для Debian
			{wxT("Linux Mint"), wxT("Clement Lefebvre"), 2006}, // Данные для Linux Mint
			{wxT("Fedora"), wxT("Warren Togami"), 2003}, // Данные для Fedora
			{wxT("Arch Linux"), wxT("Judd Vinet"), 2002}, // Данные для Arch Linux
			{wxT("Red Hat Linux"), wxT("Marc Ewing"), 1993}, // Данные для Red Hat Linux
			{wxT("Red Hat Enterprise Linux"), wxT("Marc Ewing"), 2002}, // Данные для Red Hat Enterprise Linux
			{wxT("SUSE Linux"), wxT("Roland Dyroff"), 1994}, // Данные для SUSE Linux
			{wxT("Manjaro"), wxT("Philip Müller"), 2011}, // Данные для Manjaro
			{wxT("elementary OS"), wxT("Daniel Foré"), 2011}, // Данные для elementary OS
			{wxT("Kali Linux"), wxT("Mati Aharoni"), 2013}, // Данные для Kali Linux
			{wxT("Gentoo"), wxT("Daniel Robbins"), 2000}, // Данные для Gentoo
			{wxT("Slackware"), wxT("Patrick Volkerding"), 1993} // Данные для Slackware
		};

		sqlite3_stmt* stmt; // Указатель на подготовленный SQL-запрос
		const char* insertSql = "INSERT INTO distributions(id, title, author, year) VALUES(?, ?, ?, ?);"; // SQL-запрос для вставки данных
		sqlite3_prepare_v2(db, insertSql, -1, &stmt, nullptr); // Подготавливаем запрос для выполнения

		int id = 1; // Инициализируем счётчик ID для вставки
		for (const auto& dist : distributions) { // Перебираем все дистрибутивы в векторе
			sqlite3_bind_int(stmt, 1, id++); // Привязываем значение ID к первому параметру запроса
			sqlite3_bind_text(stmt, 2, dist.title.ToUTF8(), -1, SQLITE_TRANSIENT); // Привязываем название к второму параметру
			sqlite3_bind_text(stmt, 3, dist.author.ToUTF8(), -1, SQLITE_TRANSIENT); // Привязываем автора к третьему параметру
			sqlite3_bind_int(stmt, 4, dist.year); // Привязываем год к четвёртому параметру
			sqlite3_step(stmt); // Выполняем запрос для вставки одной записи
			sqlite3_reset(stmt); // Сбрасываем состояние запроса для следующей итерации
		}
		sqlite3_finalize(stmt); // Освобождаем ресурсы, связанные с запросом
	}

	void LoadDataAsync() { // Метод для асинхронной загрузки данных в отдельном потоке
		std::thread([this]() { // Создаём новый поток для выполнения загрузки данных
			std::vector<TableFields> data; // Создаём вектор для хранения данных из базы
			sqlite3_stmt* stmt; // Указатель на подготовленный SQL-запрос

			if (sqlite3_prepare_v2(db, "SELECT * FROM distributions", -1, &stmt, nullptr) == SQLITE_OK) { // Подготавливаем запрос для выборки всех данных
				while (sqlite3_step(stmt) == SQLITE_ROW) { // Перебираем строки результата запроса
					TableFields dist; // Создаём структуру для хранения одной записи
					dist.id = sqlite3_column_int(stmt, 0); // Извлекаем ID из первой колонки
					dist.title = wxString::FromUTF8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))); // Извлекаем название из второй колонки
					dist.author = wxString::FromUTF8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2))); // Извлекаем автора из третьей колонки
					dist.year = sqlite3_column_int(stmt, 3); // Извлекаем год из четвёртой колонки
					data.push_back(dist); // Добавляем запись в вектор
				}
				sqlite3_finalize(stmt); // Освобождаем ресурсы, связанные с запросом
			}

			wxThreadEvent event(EVT_LOAD_DATA); // Создаём событие для передачи данных
			event.SetPayload(data); // Устанавливаем вектор данных как полезную нагрузку события
			wxQueueEvent(this, event.Clone()); // Отправляем копию события в основной поток
			}).detach(); // Отсоединяем поток для независимого выполнения
	}

	void OnLoadData(wxThreadEvent& event) { // Обработчик события загрузки данных
		auto data = event.GetPayload<std::vector<TableFields>>(); // Извлекаем вектор данных из события
		listCtrl->DeleteAllItems(); // Очищаем компонент списка перед обновлением

		for (const auto& dist : data) { // Перебираем все записи в векторе данных
			long index = listCtrl->InsertItem(listCtrl->GetItemCount(), wxString::Format(wxT("%d"), dist.id)); // Вставляем новую строку с ID
			listCtrl->SetItem(index, 1, dist.title); // Устанавливаем название в первую колонку
			listCtrl->SetItem(index, 2, dist.author); // Устанавливаем автора во вторую колонку
			listCtrl->SetItem(index, 3, wxString::Format(wxT("%d"), dist.year)); // Устанавливаем год в третью колонку
		}
	}
};

class MyApp : public wxApp { // Определяем класс приложения, наследуясь от wxApp
public:
	bool OnInit() override { // Переопределяем метод инициализации приложения
		setlocale(LC_ALL, ""); // Устанавливаем локаль для поддержки Unicode и кириллицы
		MyFrame* frame = new MyFrame(); // Создаём экземпляр главного окна
		frame->Show(true); // Показываем главное окно на экране
		return true; // Возвращаем true для успешной инициализации
	}
};

wxIMPLEMENT_APP(MyApp); // Регистрируем класс приложения как точку входа