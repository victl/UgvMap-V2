#include "IntegratedNavigation.h"
namespace integratednavigation
{
	
	void gettime(unsigned long & sec, unsigned long & usec)
	{
		struct timeval time_now;
		gettimeofday(&time_now, NULL);
		sec=(unsigned long)time_now.tv_sec;
		usec=(unsigned long)time_now.tv_usec;
	}


	void getGPSIMU()
	{
		double temp_Acceleration[3];
		unsigned char * p;		

		char port_name[]="/dev/ttyM2";
		SerialInterface * serial_interface=new SerialInterface();
		serial_interface->openSerial(port_name);
		serial_interface->setSerial();

		int begin = 0;
		int len;
		int i;
		unsigned char buffer[2000];
		unsigned char packet[1000];
		unsigned char * a;
		int mark = 0;
		int counter = 0;
		int kind;

	
		while(1)
		{
			if( begin==0 )
			{
				memset(buffer, 0, sizeof(buffer));
				len=read(serial_interface->fd,buffer,sizeof(buffer));			
				a=buffer;
			}


			kind=0;
			for( i=begin;i<len;i++ )
			{
				if( mark==0 && *(buffer+i)==0xAA )
				{
					mark=11; //header
					counter=4; //the lenght of the header
					p=packet;
					//n=0;
				}
				
				if( mark>0 )
				{
					*p=*(buffer+i);
					p++;
					counter--;
					//n++;
				}

				if( counter==0 && mark==11 )
				{
					if( packet[1]==0x44 && packet[2]==0x13 )
					{
						mark=13;
						counter=12+(int)packet[3];
					}
					else
					{
						//restart
						mark=0;
						p=packet;
					}
				}
				else if( counter==0 && mark==13 )
				{
					//PVA and so on
					//n;
					begin=i+1;
					if( begin==len )begin=0;
					mark=0;
					counter=0;
					kind=1;
					break;
				}
				if( i+1==len )begin=0;

			}

			if( kind==1 )
			{
				int MessageID;
				//int counter_18=18;

				long n;
				double double_n;

				MessageID=*(unsigned short *)(packet+4);

				int H=12;
				//cout<<"message:"<<MessageID<<endl;
				if( MessageID==508 )//此处类型需要添加
				{
					cout << "********************" << endl;
					//Seconds=*(double *)(packet+4+H);
					LLH[0]=*(double *)(packet+12+H);
					LLH[1]=*(double *)(packet+20+H);
					LLH[2]=*(double *)(packet+28+H);

					Velocity[1]=*(double *)(packet+36+H);
					Velocity[0]=*(double *)(packet+44+H);
					Velocity[2]=*(double *)(packet+52+H);

					Eulr[0]=*(double *)(packet+60+H);
					Eulr[1]=*(double *)(packet+68+H);
					Eulr[2]=*(double *)(packet+76+H);
					



				}
				else if( MessageID==325 )//此处类型需要添加
				{
					cout << "#######################"<< endl;
					n= *(long *)(packet+24+H);
					double_n=(double)(n);
					temp_Acceleration[0]=(double)(double_n*0.05*pow(2,-15));
					n= *(long *)(packet+20+H);
					double_n=(double)(n);
					temp_Acceleration[1]=(double)((-1)*double_n*0.05*pow(2,-15));
					n= *(long *)(packet+16+H);
					double_n=(double)(n);
					temp_Acceleration[2]=(double)(double_n*0.05*pow(2,-15));


					Acceleration[0]=Acceleration[0]+temp_Acceleration[0];
					Acceleration[1]=Acceleration[1]+temp_Acceleration[1];
					Acceleration[2]=Acceleration[2]+temp_Acceleration[2];

					

				}
				else if( MessageID==320 )//此处类型需要添加
				{
					cout << "******######********" << endl;
					for(int i=0;i<9;i++)
					{
						PositionCov[i]=*(double *)(packet+12+i*8+H);
					}
					for(int i=0;i<9;i++)
					{
						AttitudeCov[i]=*(double *)(packet+84+i*8+H);
					}
					for(int i=0;i<9;i++)
					{
						VelocityCov[i]=*(double *)(packet+156+i*8+H);
					}

				}
				else if( MessageID==622 )//此处类型需要添加
				{
					TicksPerRev=*(unsigned short *)(packet+H);

					WheelVel=*(unsigned short *)(packet+H+2);

					fWheelVel=*(unsigned long *)(packet+H+4);

					TicksPerSecond=*(unsigned long *)(packet+H+16);
				}
				kind=0;
			}
			else if( kind==2 )
			{

			}


		}


	}


	//instruction function: new a structure for states
	IntegratedNavigation::IntegratedNavigation()
	{
		data=new GPSData_t;
	}

	//destruction function: release recourese
	IntegratedNavigation::~IntegratedNavigation()
	{
		delete data;
		mycounter.Quit();
	}

	//initialize variables and mutex, and start threads
	bool IntegratedNavigation::Initialize()
	{
		//参考坐标系原点
		TripleNode orgxyz = GetOrg_xyz();
		data->org_xyz[0] = orgxyz.x;
		data->org_xyz[1] = orgxyz.y;
		data->org_xyz[2] = orgxyz.z;
		//initialize variables
		for(int i=0;i<3;i++)
		{
				Eulr[i]=0;
				LLH[i]=0;
				Velocity[i]=0;
				Acceleration[i]=0;
		}
	   
	    WheelCount = 0;
		LastWheelCount = 0;
		TicksPerRev=0;
		WheelVel=0;
		fWheelVel=0;
		TicksPerSecond=0;
		ID = 0;
		mycounter.Start();
		gettime(last_sec,last_usec);
		return true;
	}


	
	bool IntegratedNavigation::CheckLLH()
	{
		if( Eulr[0]>=400 || Eulr[0]<-400 )
		{
			cout<<"Eulr error!!!"<<endl;
			return false;
		}
		if( Eulr[1]>=400 || Eulr[1]<-400 )
		{
			cout<<"Eulr error!!!"<<endl;
			return false;
		}
		if( Eulr[2]>=400 || Eulr[2]<0 )
		{
			cout<<"Eulr error!!!"<<endl;
			return false;
		}
		if( LLH[0]<20 || LLH[0]>80 )
		{
			cout<<"LLH error!!!"<<endl;
			return false;
		}
		if( LLH[1]<80 || LLH[1]>160 )
		{
			cout<<"LLH error!!!"<<endl;
			return false;
		}
		
		return true;
	}
	
	TripleNode IntegratedNavigation::GetOrg_xyz()
	{
		double Org_llh[3]= {31.59971848,120.7682072,18.8910392};
		Org_llh[0]=Org_llh[0]*PI/180.0;
		Org_llh[1]=Org_llh[1]*PI/180.0;
		double Org_xyz[3];
		llh2xyz(Org_llh,Org_xyz);
		TripleNode xyz;
		xyz.x = Org_xyz[0];
		xyz.y = Org_xyz[1];
		xyz.z = Org_xyz[2];
		return xyz;
	}
	
	TripleNode IntegratedNavigation::GetEnu()
	{
		double temp_llh[3];
		double temp_enu[3];
		temp_llh[0]=LLH[0]*PI/180.0;
		temp_llh[1]=LLH[1]*PI/180.0;
		temp_llh[2]=LLH[2];
		llh2enu(temp_llh,Org_xyz,temp_enu);
		TripleNode xyz;
		xyz.x = temp_enu[0];
		xyz.y = temp_enu[1];
		xyz.z = temp_enu[2];
		return xyz;
	}
	
	TripleNode IntegratedNavigation::GetEulr()
	{
		TripleNode theta;
		theta.x=Eulr[0]*PI/180.0;
		theta.y=Eulr[1]*PI/180.0;
		theta.z=Eulr[2]*PI/180.0;
		return theta;
	}
	
	TripleNode IntegratedNavigation::GetAcceleration()
	{
		unsigned long sec,usec;
		gettime(sec,usec);

		double dt0=double(sec-last_sec)+double(usec-last_usec)/1000000.0;
		last_sec=sec;
		last_usec=usec;
		TripleNode a;
		a.x = Acceleration[0]/dt0;
		a.y = Acceleration[1]/dt0;
		a.z = Acceleration[2]/dt0;
		Acceleration[0]=0;
		Acceleration[1]=0;
		Acceleration[2]=0;
		return a;
	}
	
	TripleNode IntegratedNavigation::GetVelocity()
	{
		TripleNode v;
		v.x = Velocity[0];
		v.y = Velocity[1];
		v.z = Velocity[2];
		return v;
	}
	
	TripleNode IntegratedNavigation::GetCOV()
	{
		TripleNode cov;
		cov.x = PositionCov[0];
		cov.y = PositionCov[4];
		cov.z = AttitudeCov[8];
		return cov;
	}
	
	int IntegratedNavigation::GetCount()
	{
		LastWheelCount = WheelCount;
		WheelCount = mycounter.count;
		return WheelCount;
	}
	

    double IntegratedNavigation::GetImuV(TripleNode v)
	{
		return sqrt(v.x*v.x+v.y*v.y);
		
	}
	
	double IntegratedNavigation::GetDrV()
	{
		return (WheelCount - LastWheelCount)*(-0.0004851);
	}
	
	double IntegratedNavigation::GetA(TripleNode a)
	{
		return sqrt(a.x*a.x+a.y*a.y);
	}
	//main process
	bool IntegratedNavigation::Process()
	{
		ID++;
		data->ID = ID;
		getGPSIMU();
		mycounter.Read();
		if(!CheckLLH())
			return false;
		data->count = GetCount();
		data->DR_V = GetDrV();

		//ENU
		TempNode = GetEnu();
		data->ENU[0] = TempNode.x;
		data->ENU[1] = TempNode.y;
		data->ENU[2] = TempNode.x;
		//Eulr
		TempNode = GetEulr();
		data->Eulr[0] = TempNode.x;
		data->Eulr[1] = TempNode.y;
		data->Eulr[2] = TempNode.z;
		//Velocity
		TempNode = GetVelocity();
		data->Velocity[0] = TempNode.x;
		data->Velocity[1] = TempNode.y;
		data->Velocity[2] = TempNode.z;
		data->IMU_V = GetImuV(TempNode);
		//Acceleration
		TempNode = GetAcceleration();
		data->Acceleration[0] = TempNode.x;
		data->Acceleration[1] = TempNode.y;
		data->Acceleration[2] = TempNode.z;
		data->A = GetA(TempNode);
		//Cov
		TempNode = GetCOV();
		data->COV[0] = TempNode.x;
		data->COV[1] = TempNode.y;
		data->COV[2] = TempNode.z;

		return true;
	}


}
