# smart_home_module
Smart home part for skills score

# инструкция по сборке
для работы с модулем требуется:
1. установить zephyrproject и zephyrSDK следуя инструкции по ссылке:  
https://docs.zephyrproject.org/latest/develop/getting_started/index.html
2. перейти в папку zephyrproject, открыть терминал, ввести команду для клонирование репозитория:  
git clone https://github.com/AlaiziyZukerman/smart_home_module.git
3. перейти в папку smart_home_module
4. что бы построить проект введите команду:  
west build -p auto -b ESP32C3_LUATOS_CORE ./  
если используется другой контроллер следует вписать его название
5. что бы прошить микроконтроллер введите команду:  
west flash  

## интеграция в проект
- для интеграции модуля в проект требуется копировать файлы module.c и module.h  
в соответствующие папки.  
- в файле конфигурайии проекта (.conf) необходимо включить конфигурацию  
как в файле /smart_home_module/prj.conf  
- в файле .overlay необходимо добавить конфигурацию как в  
/smart_home_module/boards/esp32c3_luatos_core.conf 
- в файле CMakeList.txt включить сборку файла module.c  
- в проекте необходимо добавить #include "module.h" и перенести из /smart_home_module/src/main.c  
секции инициализации потоков и очереди, а так же всего необходимого для их работы
  
## FreeRtos
что бы портировать код в FreeRtos необходимо в module.h изменить #define FREERTOS с 0 на 1  
так же необходимо дописать имплементацию некоторых платформозависимых функций типа LOG и uart

## инструкция по работе
в проекте реализована передача данных на сервер через последовательный интерфейс uart1,  
команды и конфигурация передачи реализована через uart0.  
список поддерживаемых команд:  
- time:****  
диапазон значений от 99 до 8000 миллисекунд устанавливает время между опросами датчиков  
> time:100  
> <inf> module: command accepted: pooling time->100 msec  
- read  
начинает передачу данных на сервер
> read  
> <inf> module: command accepted: reading start
- stop  
останавливает передачу данных  
> stop  
> <inf> module: command accepted: reading stop  
- toggle  
переключает режим передачи данных на сервер:  
датчик номер 17, температура 24 градуса  
полнотекстовый: "sensor: 17; temp: 24;\r\n"  
сокращенный: "7:24;"  
> toggle  
> <inf> module: command accepted: toggle data format
- status  
возвращает статус работы модуля, формат передачи данных, и время опроса  
> status  
> <inf> module: status: reading->stop; fotmat->bin; time->100 msec;
