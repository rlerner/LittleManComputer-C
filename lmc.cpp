#include <iostream>
#include <stdexcept>
using namespace std;

class Input {
	private:
		string data;
	public:
		string get() {
			cout<<"?";
			cin>>data;
			return data;
		}
};

class Output {
	private:
		string data;
	public:
		void set(string value) {
			cout<<value<<"\n";
		}
		string get() { // Not needed
			return data;
		}
};

class Flag {
	private:
		bool state;
	public:
		void set(bool flag) {
			state = flag;
		}
		bool get() {
			return state;
		}
};

class Register {
	private:
		unsigned int data;
	public:
		void set(unsigned int value) {
			if (value<0 || value>999) {
				throw std::invalid_argument("Register Overflow attempting to set value `" + to_string(value) + "`");
			}
			data = value;
		}
		unsigned int get() {
			return data;
		}
};

class RAM {
	private:
		unsigned int data[99];
	public:
		RAM() { // Constructor to initalize RAM values
			int i;
			for (i=0;i<100;i++) {
				data[i] = 0;
			}
		}
		void set(unsigned int location, unsigned int value) {
			if (location<0 || location>99) {
				throw std::invalid_argument("RAM Location on `set`: Out of Bounds, requesting location " + to_string(location));
			}
			if (value<0 || value>999) {
				throw std::invalid_argument("RAM Value Overflow setting value `" + to_string(value) + "` to location " + to_string(location));
			}
			data[location] = value;
		}
		unsigned int get(unsigned int location) {
			if (location<0 || location>99) {
				throw std::invalid_argument("RAM Location on `get`: Out of Bounds, requesting location " + to_string(location));
			}
			return data[location];
		}
		//todo: Import would be cool like the PHP version -- later

		void dump() {
			int i;
			for (i=0;i<100;i++) {
				cout<<i<<":"<<data[i]<<"\n";
			}
		}
};

class LMC {
	private:
		Register accumulator;
		Register programCounter;
		Register instruction;
		RAM ram;
		Input input;
		Output output;
		unsigned int instructionCount = 0;
		Flag negative;
		bool log;

	public:
		LMC(RAM ramIn, Input inputIn, Output outputIn, Register programCounterIn, Register accumulatorIn, Register instructionIn, Flag negativeIn, bool logging) {
			ram = ramIn;
			input = inputIn;
			output = outputIn;
			programCounter = programCounterIn;
			instruction = instructionIn;
			accumulator = accumulatorIn;
			negative = negativeIn;
			log = logging;
			
			// Initalize flags & pointers
			negative.set(false);
			programCounter.set(0);
			accumulator.set(0);
			instruction.set(0);
		}

		void run() {
			//Initalize
			unsigned int currentInstruction = 0;
			unsigned int currentLocation = 0;
			unsigned int val;
			int value; // Do not make unsigned, it needs to wrap to negative for SUB calculations
			unsigned int acc;

			while (true) {
				
				// Get current instruction from RAM, set to instruction register
				instruction.set(ram.get(programCounter.get()));

				// Get current instruction from instruction register
				currentInstruction = instruction.get();

				// Iterate PC
				programCounter.set(programCounter.get()+1);


				// Iterate instruction counter for optimizing programs
				instructionCount++;


				// Process instructions below


				// HLT (0xx) Halt Processing
				if (currentInstruction < 100) { // HLT - 0xx
					
					logLine("HLT Encountered. Ending. " + to_string(instructionCount) + " total instruction(s).");
					return;

				// ADD (1xx) Add Memory Location to Accumulator
				} else if (currentInstruction < 200) {
					currentLocation = currentInstruction - 100;

					logLine("ADD RAM ByRef " + to_string(currentLocation) + " to accumulator");

					int val = ram.get(currentLocation);
					int acc = accumulator.get();

					if (negative.get()) {
						// If the sum of a neg accumulator + pos value is positive then invert negative flag
						acc = -acc;
					}

					if (acc+val>0) {
						negative.set(false);
					}

					accumulator.set(acc+val);

				// SUB (2xx) Subtract Memory Location from Accumulator
				} else if (currentInstruction < 300) {
					currentLocation = currentInstruction - 200;

					logLine("SUB RAM from accumulator");

					value = accumulator.get() - ram.get(currentLocation);
					negative.set(false);
					if (value<0) {
						negative.set(true);
						value = -value;
					}
					accumulator.set(value);

				// STA (3xx) Store Accumulator to RAM
				} else if (currentInstruction < 400) {
					currentLocation = currentInstruction - 300;

					logLine("STA Store accumulator to RAM ByRef " + to_string(currentLocation));

					ram.set(currentLocation,accumulator.get());
					negative.set(false); // If the accumulator isn't changing, why is this? Should be in LDA?

				// N/A (4xx) Invalid Code (acts as NOP)
				} else if (currentInstruction < 500) { 
					// Currently a NOP, nothing will be done with this cycle -- since another invalid code (903, etc) crashes
					// should 4xx crash too?

				// LDA (5xx) Load RAM value into accumulator, reset negative flag
				} else if (currentInstruction < 600) {
					currentLocation = currentInstruction - 500;

					logLine("LDA Load RAM value from ByRef " + to_string(currentLocation) + " into accumulator, unset negative flag");

					accumulator.set(ram.get(currentLocation));
					negative.set(false);

				// BRA (6xx) Branch Always
				} else if (currentInstruction < 700) {
					currentLocation = currentInstruction - 600;
					logLine("BRA Branch Always to " + to_string(currentLocation));

					programCounter.set(currentLocation);

				// BRZ (7xx) Branch if accumulator zero
				} else if (currentInstruction < 800) {
					currentLocation = currentInstruction - 700;
					logLine("BRZ Branch if Accumulator = Zero to " + to_string(currentLocation));

					if (accumulator.get()==0) {
						programCounter.set(currentLocation);
					}

				// BRP (8xx) Branch if negative flag is false
				} else if (currentInstruction < 900) {
					currentLocation = currentInstruction - 800;
					logLine("BRP Branch if Positive (Flag Negative is not set) to " + to_string(currentLocation));

					if (!negative.get()) {
						programCounter.set(currentLocation);
					}

				// INP (901) Get input, add to accumulator
				} else if (currentInstruction == 901) {
					logLine("INP");
					accumulator.set(stoi(input.get()));

				// OUT (902)
				} else if (currentInstruction == 902) {
					logLine("OUT");
					output.set(to_string(accumulator.get()));

				// Unknown opcode (should 4xx end up here?) // todo
				} else {
					throw std::invalid_argument("Invalid Opcode");
				}
			}
		}

		void logLine(string message) {
			if (log) {
				if (negative.get()) {
					cout<<"PC:"<<programCounter.get()<<" A:"<<accumulator.get()<<" N:- "<<message<<"\n";
				} else {
					cout<<"PC:"<<programCounter.get()<<" A:"<<accumulator.get()<<" N:+ "<<message<<"\n";
				}
			}
		}

};




int main(int argc, char *argv[]) {
	//RAM RAMObj;
	//LMC littleManComputer(RAMObj, new Input, new Output, new Register, new Register, new Register, new Flag);

	cout<<"LittleManComputer, C++ Version, R.Lerner 20220810\n";

	if (argc!=2) {
		cout<<"Two parameters expected, run with "<<argv[0]<<" [filename.lmc]\n\n";
		return 1;
	}
	// Would like to get a log flag from args too


	RAM RAMx;
	Input Inputx;
	Output Outputx;
	Register Register1x;
	Register Register2x;
	Register Register3x;
	Flag Flagx;

	FILE *fH;
	char line[10]; // Probably can be 4 ###\n?
	int ramLocation = 0;
	fH = fopen(argv[1],"r"); //todo: would like to do a file exists check or assign this to something besides argv inline
	if (fH==NULL) {
		perror("Can't open file"); // what is perror?
		return 1;
	} else {
		while (fgets(line,10,fH)) {
			RAMx.set(ramLocation,stoi(line));
			ramLocation++;
		}

	}
	fclose(fH);


	LMC littleManComputer(RAMx,Inputx,Outputx,Register1x,Register2x,Register3x,Flagx,true);

	littleManComputer.run();

	return 0;
}