#include <iostream>
using namespace std;

#define ZERO_FWD 64
#define ZERO_YAW 64
#define ZERO_VERT 64

#define M_TURN_STOP 0
#define M_TURN_LEFT 1
#define M_TURN_RIGHT 2

#define M_GRIP_STOP 0
#define M_GRIP_IN 1
#define M_GRIP_OUT 2

#define THR_STOP 0
#define THR_LEFT 1
#define THR_RIGHT 2

#define CAM_Z_STOP 0
#define CAM_Z_LEFT 1
#define CAM_Z_RIGHT 2

#define CAM_Y_STOP 0
#define CAM_Y_UP 1
#define CAM_Y_DOWN 2

LPCSTR sPortName;
HANDLE hSerial;
DCB dcbSerialParams = {0};

struct _ControlPadData{
    unsigned char fwd;
    unsigned char yaw;
    unsigned char vert;
    unsigned char m_turn;
    unsigned char m_grip;
    unsigned char thr;
    bool lights;
    bool switch_cam;
    unsigned char cam_z;
    unsigned char cam_y;
    bool additional;
    void set_default();
    void display();
} ControlPadData;

//Устанавливает поля структуры ControlPadData в исходное состояние (вызывается при ошибках данных от пульта)
void _ControlPadData::set_default()
{
    fwd = ZERO_FWD;
    yaw = ZERO_YAW;
    vert = ZERO_VERT;
    m_turn = M_TURN_STOP;
    m_grip = M_GRIP_STOP;
    thr = THR_STOP;
    lights = 0;
    switch_cam = 0;
    cam_z = CAM_Z_STOP;
    cam_y = CAM_Y_STOP;
    additional = 0;
}

void _ControlPadData::display()
{
    cout<<"fwd = "<<(unsigned int)fwd<<endl;
    cout<<"yaw = "<<(unsigned int)yaw<<endl;
    cout<<"vert = "<<(unsigned int)vert<<endl;
    cout<<"m_turn = "<<(unsigned int)m_turn<<endl;
    cout<<"m_grip = "<<(unsigned int)m_grip<<endl;
    cout<<"thr = "<<(unsigned int)thr<<endl;
    cout<<"lights = "<<lights<<endl;
    cout<<"switch_cam = "<<switch_cam<<endl;
    cout<<"cam_z = "<<(unsigned int)cam_z<<endl;
    cout<<"cam_y = "<<(unsigned int)cam_y<<endl;
    cout<<"additional = "<<additional<<endl;
    cout<<endl<<endl<<endl;
}

int UpdatePadData(unsigned char ch)
{
    static int byte_n = -1;
    if (ch & (1)) byte_n = 0; //Если младший бит принятого байта равен 1, то это нулевой байт пакета.
    switch (byte_n)     //далее обрабатываем принятый байт в зависимости от того, какой он по порядку
    {
        case 0: 
            ControlPadData.fwd = (ch>>1);
            byte_n++;
            break;
        case 1:
            ControlPadData.yaw = (ch>>1); 
            byte_n++;   
            break;
        case 2:
            ControlPadData.vert = (ch>>1); 
            byte_n++;
            break;
        case 3:   
        {      //Выделяем необходимые биты из байта:
            unsigned char turn = (ch>>6),           //6й и 7й биты
                          grip = ((ch>>4) & 3),     //4й и 5й
                          thrust = ((ch>>2) & 3);   //2й и 3й
            //устанавливаем необходимое значение для:
            // -поворота манипулятора
            switch (turn)                           
            {
                case 2: 
                    ControlPadData.m_turn = M_TURN_LEFT;
                    break;
                case 3:
                    ControlPadData.m_turn = M_TURN_RIGHT;
                    break;
                default:
                    ControlPadData.m_turn = M_TURN_STOP;
            }
            // -сжатия/расжатия манипулятора
            switch (grip)
            {
                case 2: 
                    ControlPadData.m_grip = M_GRIP_IN;
                    break;
                case 3:
                    ControlPadData.m_grip = M_GRIP_OUT;
                    break;
                default:
                    ControlPadData.m_grip = M_GRIP_STOP;
            }
            // -боковых двигателей
            switch (thrust)
            {
                case 2: 
                    ControlPadData.thr = THR_LEFT;
                    break;
                case 3:
                    ControlPadData.m_grip = THR_RIGHT;
                    break;
                default:
                    ControlPadData.m_grip = THR_STOP;
            }
            // 1-й бит отвечает за нажатие кнопки освещения:
            if (ch & 1)          //если он установлен, значение переменной осввещения инвертируется
                ControlPadData.lights = !ControlPadData.lights; 
            byte_n++;
            break;  
        }
        case 4:                                  
        {    //выделяем необходимые биты из байта:
            unsigned char camZ = ((ch>>4) & 3),   //4й и 5й биты
                          camY = ((ch>>2) & 3);   //2й и 3й
            //устанавливаем необходимое значение для:
            // -поворота камеры по оси Z
            switch (camZ)                           
            {
                case 2: 
                    ControlPadData.cam_z = CAM_Z_LEFT;
                    break;
                case 3:
                    ControlPadData.cam_z = CAM_Z_RIGHT;
                    break;
                default:
                    ControlPadData.cam_z = CAM_Z_STOP;
            }
            // -поворота камеры по оси Y
            switch (camY)                           
            {
                case 2: 
                    ControlPadData.cam_y = CAM_Y_UP;
                    break;
                case 3:
                    ControlPadData.cam_y = CAM_Y_DOWN;
                    break;
                default:
                    ControlPadData.cam_y = CAM_Y_STOP;
            }
            // 6-й бит отвечает за нажатие кнопки смены камеры:
            if (ch & (1<<6))          //если он установлен, происходит перекючение камеры
                ControlPadData.switch_cam = !ControlPadData.switch_cam; 
            // 1-й бит отвечает за нажатие дополнительной кнопки:
            if (ch & 1)          //если он установлен, это отмечается 
                ControlPadData.additional = !ControlPadData.additional;     
            byte_n++;
            break;
        }
        default: 
            ControlPadData.set_default();          
    }
    return(byte_n);
}

void COM_init()
{
    sPortName = "COM4";
    hSerial = ::CreateFile(sPortName,GENERIC_READ,0,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
    if(hSerial==INVALID_HANDLE_VALUE)
    {
        if(GetLastError()==ERROR_FILE_NOT_FOUND)
    {
        cout << "serial port does not exist.\n";
    }
    cout << "some other error occurred.\n";
    }
    dcbSerialParams.DCBlength=sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams))
    {
        cout << "getting state error\n";
    }
    dcbSerialParams.BaudRate=CBR_9600;
    dcbSerialParams.ByteSize=8;
    dcbSerialParams.StopBits=TWOSTOPBITS;
    dcbSerialParams.Parity=EVENPARITY;
    if(!SetCommState(hSerial, &dcbSerialParams))
    {
        cout << "error setting serial port state\n";
    }
}

void ReadCOM(int iterations)
{
      DWORD iSize;
      unsigned char sReceivedChar;
      while (iterations)
      {
            ReadFile(hSerial, &sReceivedChar, 1, &iSize, 0);  // получаем 1 байт
            if (iSize > 0)   // если что-то принято, выводим
            {
                if(UpdatePadData(sReceivedChar) == 5) //то есть, если последний обработанный байт был 4-м в пакете
                {
                   // ControlPadData.display();
                }
                //iterations--;
            }
                
            
      }
}

