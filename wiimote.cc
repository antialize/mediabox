#include <map>
#include <bluetooth/bluetooth.h>
extern "C" {
#include <cwiid.h>
}
#include <math.h>
#include "threading.hh"
#include "input.hh"
#define PI 3.14159265358979323

using namespace std;
template <typename T> struct vector {
    T x, y, z;
    vector(): x(0),y(0),z(0) {};
    vector(T a,T b,T c): x(a),y(b),z(c) {};
    vector operator +(const vector & o) const {return vector(x+o.x,y+o.y,z+o.z);}
    vector operator -(const vector & o) const {return vector(x-o.x,y-o.y,z-o.z);}
    vector operator *(const vector & o) const {return vector(x*o.x,y*o.y,z*o.z);}
    vector operator /(const vector & o) const {return vector(x/o.x,y/o.y,z/o.z);}
};

class WiiMote {
public:
	InputStack * i;
	static Mutex mutex;
    static map<cwiid_wiimote_t *,WiiMote *> instances;
    cwiid_wiimote_t * m;

    vector<double> one;
    vector<double> zero;
    vector<double> a;
    double roll;
    double pitch;
	uint16_t oldbtns;
    void setAcc(struct cwiid_acc_mesg *mesg) {
        a = (vector<double>(mesg->acc[0],mesg->acc[1],mesg->acc[2]) - zero) / (one-zero);
        roll = atan(a.x / a.z);
        if(a.z <= 0.0) roll += PI * (a.x>0.0?1:-1);
        roll *= -1;
        pitch = atan( a.y / a.z * cos(roll) );
    }

	void mapBtn(uint16_t btns, uint16_t mask, int btn) {
		if(btns & mask && !(oldbtns & mask))
			i->triggerSpecialKey(btn);
	}
	
	void setButtons(uint16_t btns) {
		mapBtn(btns,CWIID_BTN_LEFT,InputListener::left);
		mapBtn(btns,CWIID_BTN_RIGHT,InputListener::right);
		mapBtn(btns,CWIID_BTN_UP,InputListener::up);
		mapBtn(btns,CWIID_BTN_DOWN,InputListener::down);
		mapBtn(btns,CWIID_BTN_A,InputListener::enter);
		mapBtn(btns,CWIID_BTN_B,InputListener::left);
		mapBtn(btns,CWIID_BTN_HOME,InputListener::escape);
		mapBtn(btns,CWIID_BTN_1,InputListener::pageup);
		mapBtn(btns,CWIID_BTN_2,InputListener::pagedown);
		oldbtns = btns;
	}

	int z;
	bool first;
	int oldbtn;
	void accKeys() {
		int btn;
		int rep;
		if(pitch > 1.0) {
			btn=InputListener::pagedown; rep=8;
		} else if(pitch > 0.2) {
			btn=InputListener::down; 
			rep=(int)(1.0+7.0*(1.0-pitch)/0.8);
		} else if(pitch < -1.0) {
			btn=InputListener::pageup; rep=8;
		} else if(pitch < -0.2) {
			btn=InputListener::up; 
			rep=(int)1.0+7.0*(pitch+1.0)/0.8;
		}  else {
			z=0;
			first=true;
			return;
		}
		if(btn != oldbtn) {
			z=0;
			first=true;
			oldbtn=btn;
		}
		if(z == 0) {
			printf("trigger\n");
			i->triggerSpecialKey(btn);
		}
		if((!first && z >= rep) || (first && z >= 15)) {
			i->triggerSpecialKey(btn);
			z=0;
			first=false;
		}
		++z;
	}
	
    static void callback(cwiid_wiimote_t * m,
                         int count, union cwiid_mesg * msgs
                         ,timespec * time) {
		mutex.lock();
        WiiMote * x = instances[m];
		mutex.unlock();

        for(int i=0; i < count; ++i) {
            switch (msgs[i].type) {
            case CWIID_MESG_ACC:
                x->setAcc(&msgs[i].acc_mesg);
				break;
			case CWIID_MESG_BTN:
				x->setButtons(msgs[i].btn_mesg.buttons);
				break;
			case CWIID_MESG_ERROR:
				mutex.lock();
				delete x;
				mutex.unlock();
				break;
            }
        }

    };

    WiiMote(InputStack * _): i(_) {
        cwiid_wiimote_t * m = cwiid_connect(BDADDR_ANY,CWIID_FLAG_MESG_IFC);
        if(!m) throw false;
		mutex.lock();
        instances[m] = this;
		mutex.unlock();
        unsigned char buf[7];
        if(cwiid_read(m, CWIID_RW_EEPROM, 0x16, 7, buf)) throw false;

		zero = vector<double>( (double)buf[0], (double)buf[1], (double)buf[2]);
        one = vector<double>( (double)buf[4], (double)buf[5], (double)buf[6]);

        cwiid_set_mesg_callback(m,callback);

        unsigned char rpt_mode = CWIID_RPT_STATUS |
            CWIID_RPT_BTN | CWIID_RPT_ACC;
        cwiid_command(m, CWIID_CMD_RPT_MODE, rpt_mode);
        cwiid_command(m, CWIID_CMD_STATUS, 0);
        cwiid_command(m, CWIID_CMD_LED, CWIID_LED4_ON | CWIID_LED1_ON );
  
		printf("New wiimote attached\n");
	}

    ~WiiMote() {
		mutex.lock();
        cwiid_disconnect(m);
        instances.erase(m);
        m=NULL;
		mutex.unlock();
		printf("Wiimote detached\n");
    }
};

Mutex WiiMote::mutex;
map<cwiid_wiimote_t *,WiiMote *> WiiMote::instances;

class WiiMoteInputHandler: public InputHandler {
public:
	InputStack * i;

	class ConnectThread: public Thread<ConnectThread> {
	public:
		volatile bool die;
		WiiMoteInputHandler * h;
		ConnectThread(WiiMoteInputHandler * _): die(false), h(_) {};
		void run() {
			while(!die) {
				try {
					new WiiMote(h->i);
				} catch(bool x) {
				}
			}
		}
	};
	
	class AccThread: public Thread<AccThread> {
	public:
		volatile bool die;
		AccThread(): die(false) {};
		void run() {
			while(!die) {
				usleep(50000);
				WiiMote::mutex.lock();
				for(map<cwiid_wiimote_t *,WiiMote *>::iterator x = 
						WiiMote::instances.begin();
					x != WiiMote::instances.end(); ++x)
					x->second->accKeys();
				WiiMote::mutex.unlock();
			}
		}
	};

	ConnectThread con;
	AccThread acc;
	
	WiiMoteInputHandler(InputStack * _): i(_), con(this) {
		con.start();
		acc.start();
	}

	~WiiMoteInputHandler() {
		acc.die = con.die = true;
		acc.join();
		con.join();
	}
};


InputHandler * createWiimoteInputHandler(InputStack * i) {
	return new WiiMoteInputHandler(i);
}
