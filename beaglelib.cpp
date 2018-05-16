// beaglebone用　GPIOサポートユーティリティ
// リリース時、-DNDEBUG

#include "beaglelib.h"

bool is_port( int n ){	//Is gpio n already  
	string port = "/sys/class/gpio/gpio";
	port += to_string( n );
	
	ifstream ifs( port );
	return ifs.good();
}

bool gpio_export( int n ){
	if(!is_port( n )){
		ofstream ofs("/sys/class/gpio/export");
		ofs << to_string( n );
		ofs.close();
		return 1;
	}
	else{ return 0; }
}

void gpio_unexport( int n ){
	ofstream ofs("/sys/class/gpio/unexport");
	ofs << to_string( n );	
}

void gpio_init_out( int n, int val){
	string port = "/sys/class/gpio/gpio";
	port += to_string( n );

	if(!is_port( n )){ gpio_export(n); }
	
	ofstream ofs;
	ofs.open(port+"/direction");
	ofs << "out";
	ofs.close();
		
	ofs.open(port+"/value");
	ofs << to_string( val );
	ofs.close();
}

void gpio_init_in( int n, int edge){
	assert( edge >= 1 && edge <= 4 );
	
	string port = "/sys/class/gpio/gpio";
	port += to_string( n );

	if(!is_port( n )){ gpio_export(n); }
	
	ofstream ofs;
	ofs.open(port+"/direction");
	ofs << "in";
	ofs.close();

	string cedge;
	switch(edge){
	case 1:
		cedge = "rising"; break;
	case 2:
		cedge = "falling"; break;
	case 3:
		cedge = "both"; break;
	case 4:
		cedge = "none"; break;
	default:
		cerr << "edge argment error" << endl;
	}	
	ofs.open(port+"/edge");
	ofs << cedge;
	ofs.close();
}

void gpio_data_set( int n, int val){
	if(!is_port( n )){
		cerr << "port not exist" << endl;
		return;
	}
	
	string port = "/sys/class/gpio/gpio";
	port += to_string( n );
	ofstream ofs;
	
	ofs.open(port+"/value");
	ofs << to_string( val );
	ofs.close();
}

int gpio_data_get( int n ){
	if(!is_port( n )){
		cerr << "port not exist" << endl;
		return false;
	}

	string port = "/sys/class/gpio/gpio";
	port = port + to_string( n ) + "/value";
	ifstream ifs(port);
	string in;
	ifs >> in;
	
	return stoi(in);
}

