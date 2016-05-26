


#include "stdafx.h"
#include<iostream>
#include<conio.h>
#include<string>
#include<iomanip>
#include<vector>
#include<windows.h>
#include<queue>
#include<sstream>
#include<fstream>
#include<list>
#include "rapidxml.hpp"


using namespace std;
using namespace rapidxml;

//GLOBAL DATA VARIABLES
int Process_count = 0;
///////////////////////

//GLOBAL FUNCTIONS

//Calculate the priority of process
int calculate_priority(string spriority)
{
	if (spriority == "High" || spriority == "high")
	{
		return 3;
	}
	else if (spriority == "Medium" || spriority == "medium")
	{
		return 2;
	}
	else if (spriority == "Low" || spriority == "low")
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

//CHECK THE CORRESPONDING INSTRUCTION TO BE EXECUTED
int typeoins(string token)
{
	if (token.find("DISK") != std::string::npos || token.find("SCREEN") != std::string::npos || token.find("PRINTER") != std::string::npos || token.find("KEYBOARD") != std::string::npos || token.find("FILE") != std::string::npos)
	{
		return 3;
	}
	else if (token.find("COMPUTE") != std::string::npos)
	{
		return 1;
	}
	else if (token.find("MEMORY") != std::string::npos)
	{
		return 2;
	}
	else if (token.find("breakpoint") != std::string::npos)
	{
		return 4;
	}
	else
	{
		return 0;
	}
}

///////////////////////

//ATTRIBUTES FOR The optimization criterion

struct attributes
{
	int Completion_Time;
	int Burst_Time;
	int Arrival_Time;
	int Turnaround_Time;
	int Waiting_Time;
	int pid;
	int Start_Time;
	int flag;
	attributes()
	{
		Completion_Time = 0;
		Burst_Time = 0;
		Arrival_Time = 0;
		Turnaround_Time = 0;
		Waiting_Time = 0;
		Start_Time = -1;
		pid = 0;
		flag = -1;
	}
};
struct device
{
	int device_id;
	int status;
};


//CLASS OF SYSTEM CONFIGURATION
struct CFG
{
	string Policy;
	string prmpt;
	int time_slice;
	int Disk_no;
	int Printer_no;
	device * Printer;
	device * Disk;

	CFG()
	{
		Disk_no = 0;
		Printer_no = 0;
		time_slice = 0;
	}
	void setter_CFG(string a, string b, int d, int p, int ts)
	{
		Policy = a;
		prmpt = b;
		Disk_no = d;
		Printer_no = p;
		time_slice = ts;
		Printer = new device[Printer_no];
		Disk = new device[Disk_no];

	}
	void setter_CFG_timeslice(int ts)
	{
		time_slice = ts;
	}
	void getter_CFG()
	{
		cout << "Scheduling Policy : " << Policy << endl;
		cout << "Preemptive : " << prmpt << endl;
		cout << "No.of Disks : " << Disk_no << endl;
		cout << "No. of Printers : " << Printer_no << endl;
		cout << "Time Slice(If applicable it will be > 0) : " << time_slice << endl;
	}

} SYSTEM_CFG;



//PARSER FOR GETTING SYSTEM CONFIGURATION
CFG Parse_CFG(xml_node<> *root_node)
{

	xml_node<> *cfg_node = root_node->first_node();
	CFG obj;

	for (xml_node<> *NODE = cfg_node->first_node(); NODE; NODE = NODE->next_sibling())
	{
		if (strcmp(NODE->name(), "SchedulingPolicy") == 0)
		{
			obj.Policy = NODE->value();
		}
		else if (strcmp(NODE->name(), "Preemptive") == 0)
		{
			obj.prmpt = NODE->value();
		}
		else if (strcmp(NODE->name(), "NumberofDisks") == 0)
		{
			obj.Disk_no = atoi(NODE->value());
		}
		else if (strcmp(NODE->name(), "NumberofPrinters") == 0)
		{
			obj.Printer_no = atoi(NODE->value());
		}
		else if (strcmp(NODE->name(), "TimeSlice") == 0)
		{
			if (NODE->value() == NULL)
			{
				obj.time_slice = 0;
			}
			else
			{
				obj.time_slice = atoi(NODE->value());
			}
		}
	}
	return obj;
}


//CLASS OF PROCESS

struct PROCESS
{
	int pid;
	int arrival_time;
	int status;
	int burst_time;
	int priority;
	int blocked_time;
	string type;
	string code;
	queue<string> q_code;

	PROCESS()
	{
		pid = 0;
		arrival_time = 0;
		blocked_time = 0;
	}

	void fillcode(string code)
	{
		int pos = 0;
		string token;
		string delimiter = "\n";
		while ((pos = code.find(delimiter)) != std::string::npos)
		{
			token = code.substr(0, pos);
			q_code.push(token);
			code.erase(0, pos + delimiter.length());
		}
		q_code.pop();
	}
	int calculate_burst(string code)
	{
		int burst_time = 0;
		int pos = 0;
		string token;
		string delimiter = "\n";
		while ((pos = code.find(delimiter)) != std::string::npos)
		{
			token = code.substr(0, pos);
			if (typeoins(token) <= 3)
			{
				burst_time += typeoins(token);
			}
			code.erase(0, pos + delimiter.length());
		}
		return burst_time;
	}


	void setter_PROCESS(int pid1, int arrival_time1, int priority1, string type1, string code1)
	{
		pid = pid1;
		arrival_time = arrival_time1;
		priority = priority1;
		type = type1;
		code = code1;
		fillcode(code1);
		burst_time = calculate_burst(code);
	}
	void getter_PROCESS()
	{
		cout << "\nPROCESS ID : " << pid << endl;
		cout << "\nArrival Time : " << arrival_time << "\nPriority : " << priority << "\nType : " << type << "\nBurst Time : " << burst_time << "\nCODE : " << code << endl;
	}
}
*Proccess_datum;

bool PROCESS_SORT(PROCESS& a, PROCESS& b)
{
	return a.burst_time < b.burst_time;
}

bool PROCESS_SORT_PS(PROCESS& a, PROCESS& b)
{
	if (a.priority != b.priority) return a.priority > b.priority;
	else return a.arrival_time < b.arrival_time;	
}

//PARSER FOR GETTING PROCESS ATTRIBUTES
void Parse_PROCESS(xml_node<> *root_node)
{
	xml_node<> *process_node = root_node->first_node()->next_sibling();
	int index = 0;
	int pid = 0;
	int arrival_time = 0;
	int priority = 0;
	string type = "x";
	string code = "x";
	for (xml_node<> *NODE = process_node->first_node(); NODE; NODE = NODE->next_sibling())
	{
		pid = 0;
		arrival_time = 0;
		priority = 0;
		type = "x";
		code = "x";

		pid = atoi(NODE->first_attribute()->value());
		for (xml_node<> *NODE2 = NODE->first_node(); NODE2; NODE2 = NODE2->next_sibling())
		{
			if (strcmp(NODE2->name(), "ArrivalTime") == 0)
			{
				arrival_time = atoi(NODE2->value());
			}
			else if (strcmp(NODE2->name(), "Priority") == 0)
			{
				priority = calculate_priority(NODE2->value());
			}
			else if (strcmp(NODE2->name(), "Type") == 0)
			{
				type = NODE2->value();
			}
			else if (strcmp(NODE2->name(), "code") == 0)
			{
				code = NODE2->value();
			}
		}
		Proccess_datum[index].setter_PROCESS(pid, arrival_time, priority, type, code);
		index++;
	}
}

//FIRST COME FIRST SERVE SCHEDULING POLICY 
void FCFS()
{
	queue<PROCESS> ready_queue;
	queue<PROCESS> exit_queue;
	list<PROCESS> blocked_queue;
	PROCESS running_process;
	PROCESS dup_running_process;
	list<string> execution_order;
	ostringstream buffer;
	attributes* attr;
	attr = new attributes[::Process_count];
	int time = 0;

	//SORT ON THE BASIS OF ARRIVAL TIME
	for (int i = 0; i < ::Process_count - 1; i++)
	{
		for (int j = 0; j < ::Process_count - 1; j++)
		{
			if (Proccess_datum[j].arrival_time>Proccess_datum[j + 1].arrival_time)
			{
				PROCESS temp = Proccess_datum[j];
				Proccess_datum[j] = Proccess_datum[j + 1];
				Proccess_datum[j + 1] = temp;
			}
		}
	}
	for (int loop = 0; loop < ::Process_count; loop++)
	{
		attr[loop].Arrival_Time = Proccess_datum[loop].arrival_time;
		attr[loop].pid = Proccess_datum[loop].pid;
		attr[loop].Burst_Time = Proccess_datum[loop].burst_time;
		ready_queue.push(Proccess_datum[loop]);
	}

	while (!ready_queue.empty() || !blocked_queue.empty())
	{
		time++;
		if (ready_queue.empty() && !blocked_queue.empty())
		{
			time += 2;
			blocked_queue.front().q_code.pop();
			ready_queue.push(blocked_queue.front());

			buffer << "Ready queue is empty unblocking from blocked queue Process : " << ready_queue.front().pid;
			execution_order.push_back(buffer.str().c_str());
			buffer.str("");
			buffer.clear();

			blocked_queue.pop_front();
		}
		running_process = ready_queue.front();
		dup_running_process = running_process;
		for (int loop = 0; loop < ::Process_count; loop++)
		{
			if (attr[loop].pid == dup_running_process.pid && attr[loop].Start_Time == -1)
			{
				attr[loop].Start_Time = time - 1;
			}
		}
		ready_queue.pop();
		while (!running_process.q_code.empty())
		{
			time++;
			if (!blocked_queue.empty())
			{

				if (blocked_queue.front().blocked_time <= 0)
				{
					buffer << "I/O done. blocking instruction : " << blocked_queue.front().q_code.front() << " executed !! Process " << blocked_queue.front().pid << " unblocked and moved back to ready queue from blocked queue.";
					execution_order.push_back(buffer.str().c_str());
					buffer.str("");
					buffer.clear();
					blocked_queue.front().q_code.pop();
					ready_queue.push(blocked_queue.front());
					blocked_queue.pop_front();
				}
				if (!blocked_queue.empty())
				{
					for (list<PROCESS>::iterator itr = blocked_queue.begin(); itr != blocked_queue.end(); itr++)
					{
						(*itr).blocked_time--;
					}
				}
			}
			int instype = typeoins(running_process.q_code.front());
			if (instype == 2 || instype == 1)
			{
				buffer << "Instrution : " << running_process.q_code.front() << " for Process : " << running_process.pid << " EXECUTED";
				execution_order.push_back(buffer.str().c_str());
				buffer.str("");
				buffer.clear();
				running_process.q_code.pop();
			}
			else if (instype == 3)
			{
				buffer << "BLOCKING Instrution : " << running_process.q_code.front() << " for Process : " << running_process.pid;
				execution_order.push_back(buffer.str().c_str());
				buffer.str("");
				buffer.clear();

				running_process.blocked_time = 3;
				blocked_queue.push_back(running_process);
				break;

			}
			else if (instype == 4)
			{
				cout << "\nBREAKPOINT !!\n\n";
				running_process.q_code.pop();
				cin.ignore();
				queue<PROCESS> dup_ready_queue = ready_queue;
				queue<PROCESS> dup_exit_queue = exit_queue;

				cout << "\nProcesses currently running : " << running_process.pid;
				cout << "\nProcesses in Ready state(queue) : ";
				while (!dup_ready_queue.empty())
				{
					cout << dup_ready_queue.front().pid << " ";
					dup_ready_queue.pop();
				}
				cout << "\nProcesses in Exit state(queue) : ";
				while (!dup_exit_queue.empty())
				{
					cout << dup_exit_queue.front().pid << " ";
					dup_exit_queue.pop();
				}
				cout << "\nProcesses in Blocked(waiting) state(queue) : ";
				for (list<PROCESS>::iterator itr = blocked_queue.begin(); itr != blocked_queue.end(); itr++)
				{
					cout << (*itr).pid << " ";
				}
				cin.ignore();
				cout << "\nThe order of execution in which the code for each process has been executed till now :-\n";
				for (list<string>::iterator itr_string = execution_order.begin(); itr_string != execution_order.end(); itr_string++)
				{
					cout << (*itr_string);
					cout << endl;
					cin.ignore();
				}
			}
		}
		if (running_process.q_code.empty())
		{
			buffer << "PROCESS : " << dup_running_process.pid << " COMLETED ITS EXECUTION. PUSHED INTO EXIT QUEUE";
			execution_order.push_back(buffer.str().c_str());
			buffer.str("");
			buffer.clear();
			for (int loop = 0; loop < ::Process_count; loop++)
			{
				if (attr[loop].pid == dup_running_process.pid)
				{
					attr[loop].Completion_Time = time;
				}
			}

			exit_queue.push(dup_running_process);
		}
		if (ready_queue.empty() && blocked_queue.empty())
		{
			break;
		}
	}
	cout << "\nAll processes have been succesfully executed." << endl;
	cin.ignore();
	system("cls");
	int choice;
	cout << "\nPress 1 to review the order of execution in which the code for each process has been executed : ";
	cin >> choice;
	if (choice == 1)
	{
		cout << "\nThe order of execution in which the code for each process has been executed :-\n";
		for (list<string>::iterator itr_string = execution_order.begin(); itr_string != execution_order.end(); itr_string++)
		{
			cout << (*itr_string);
			cout << endl;
			cin.ignore();
		}
	}
	cin.ignore();
	system("cls");
	int total_waiting_time = 0;
	cout << "\nOptimization criteria :\n\n";
	cout << "PROCESS_ID\tBurst_time\tCompletion_time\tTurnaround_time\tWaiting_time\n";
	for (int loop = 0; loop < ::Process_count; loop++)
	{
		cout << attr[loop].pid << "\t\t" << attr[loop].Burst_Time;
		cout << "\t\t" << attr[loop].Completion_Time;
		attr[loop].Turnaround_Time = attr[loop].Completion_Time - attr[loop].Arrival_Time;
		cout << "\t\t" << attr[loop].Turnaround_Time;
		//attr[loop].Waiting_Time = attr[loop].Start_Time - attr[loop].Arrival_Time;
		attr[loop].Waiting_Time = attr[loop].Completion_Time - (attr[loop].Arrival_Time + attr[loop].Burst_Time);
		cout << "\t\t" << attr[loop].Waiting_Time << endl;
		total_waiting_time += attr[loop].Waiting_Time;
	}
	cout << "\nTotal time : " << time << endl;
	cout << "Average waiting time : " << (total_waiting_time / ::Process_count) << endl;
	cin.ignore();
}




//ROUND ROBIN SCHEDULING POLICY
void Round_Robin()
{
	queue<PROCESS> ready_queue;
	queue<PROCESS> exit_queue;
	list<PROCESS> blocked_queue;
	PROCESS running_process;
	PROCESS dup_running_process;
	list<string> execution_order;
	ostringstream buffer;
	attributes* attr;
	attr = new attributes[::Process_count];
	int total_time = 0;
	int time_lapse = 0;
	//SORT ON THE BASIS OF ARRIVAL TIME
	for (int i = 0; i < ::Process_count - 1; i++)
	{
		for (int j = 0; j < ::Process_count - 1; j++)
		{
			if (Proccess_datum[j].arrival_time>Proccess_datum[j + 1].arrival_time)
			{
				PROCESS temp = Proccess_datum[j];
				Proccess_datum[j] = Proccess_datum[j + 1];
				Proccess_datum[j + 1] = temp;
			}
		}
	}
	for (int loop = 0; loop < ::Process_count; loop++)
	{
		attr[loop].Arrival_Time = Proccess_datum[loop].arrival_time;
		attr[loop].pid = Proccess_datum[loop].pid;
		attr[loop].Burst_Time = Proccess_datum[loop].burst_time;
		ready_queue.push(Proccess_datum[loop]);
	}
	int time2 = 0;
	//SYSTEM_CFG.time_slice   time_lapse  total_time
	while (!ready_queue.empty() || !blocked_queue.empty())
	{
		//total_time++;
		time2++;
		if (ready_queue.empty() && !blocked_queue.empty())
		{
			total_time += 2;
			time2 += 2;
			blocked_queue.front().q_code.pop();
			ready_queue.push(blocked_queue.front());

			buffer << "Ready queue is empty unblocking from blocked queue Process : " << ready_queue.front().pid;
			execution_order.push_back(buffer.str().c_str());
			buffer.str("");
			buffer.clear();

			blocked_queue.pop_front();
		}
		running_process = ready_queue.front();
		dup_running_process = running_process;
		for (int loop = 0; loop < ::Process_count; loop++)
		{
			if (attr[loop].pid == dup_running_process.pid && attr[loop].Start_Time == -1)
			{
				attr[loop].Start_Time = time2 - 1;
			}
		}
		ready_queue.pop();
		time_lapse = 0;
		while (time_lapse != SYSTEM_CFG.time_slice && !running_process.q_code.empty())
		{
			time2++;
			total_time++;
			time_lapse++;
			if (!blocked_queue.empty())
			{
				if (blocked_queue.front().blocked_time <= 0)
				{
					buffer << "I/O done. blocking instruction : " << blocked_queue.front().q_code.front() << " executed !! Process " << blocked_queue.front().pid << " unblocked and moved back to ready queue from blocked queue.";
					execution_order.push_back(buffer.str().c_str());
					buffer.str("");
					buffer.clear();
					blocked_queue.front().q_code.pop();
					ready_queue.push(blocked_queue.front());
					blocked_queue.pop_front();
				}
				if (!blocked_queue.empty())
				{
					for (list<PROCESS>::iterator itr = blocked_queue.begin(); itr != blocked_queue.end(); itr++)
					{
						(*itr).blocked_time--;
					}
				}
			}
			int instype = typeoins(running_process.q_code.front());
			if (instype == 2 || instype == 1)
			{
				buffer << "Instrution : " << running_process.q_code.front() << " for Process : " << running_process.pid << " EXECUTED";
				execution_order.push_back(buffer.str().c_str());
				buffer.str("");
				buffer.clear();
				running_process.q_code.pop();
			}
			else if (instype == 3)
			{
				buffer << "BLOCKING Instrution : " << running_process.q_code.front() << " for Process : " << running_process.pid;
				execution_order.push_back(buffer.str().c_str());
				buffer.str("");
				buffer.clear();

				running_process.blocked_time = 3;
				blocked_queue.push_back(running_process);
				break;
			}
			else if (instype == 4)
			{
				cout << "\nBREAKPOINT !!\n\n";
				running_process.q_code.pop();
				cin.ignore();

				queue<PROCESS> dup_ready_queue = ready_queue;
				queue<PROCESS> dup_exit_queue = exit_queue;

				cout << "\nProcesses currently running : " << running_process.pid;
				cout << "\nProcesses in Ready state(queue) : ";
				while (!dup_ready_queue.empty())
				{
					cout << dup_ready_queue.front().pid << " ";
					dup_ready_queue.pop();
				}
				cout << "\nProcesses in Exit state(queue) : ";
				while (!dup_exit_queue.empty())
				{
					cout << dup_exit_queue.front().pid << " ";
					dup_exit_queue.pop();
				}
				cout << "\nProcesses in Blocked(waiting) state(queue) : ";
				for (list<PROCESS>::iterator itr = blocked_queue.begin(); itr != blocked_queue.end(); itr++)
				{
					cout << (*itr).pid << " ";
				}
				cin.ignore();
				cout << "\nThe order of execution in which the code for each process has been executed till now :-\n";
				for (list<string>::iterator itr_string = execution_order.begin(); itr_string != execution_order.end(); itr_string++)
				{
					cout << (*itr_string);
					cout << endl;
					cin.ignore();
				}
			}
		}
		if (running_process.q_code.empty())
		{
			buffer << "PROCESS : " << dup_running_process.pid << " COMLETED ITS EXECUTION. PUSHED INTO EXIT QUEUE";
			execution_order.push_back(buffer.str().c_str());
			buffer.str("");
			buffer.clear();
			for (int loop = 0; loop < ::Process_count; loop++)
			{
				if (attr[loop].pid == dup_running_process.pid)
				{
					attr[loop].Completion_Time = total_time;
				}
			}

			exit_queue.push(dup_running_process);
		}
		else if (time_lapse == SYSTEM_CFG.time_slice)
		{
			ready_queue.push(running_process);
		}
		if (ready_queue.empty() && blocked_queue.empty())
		{
			break;
		}
	}
	cout << "\nAll processes have been succesfully executed." << endl;
	cin.ignore();
	system("cls");
	int choice;
	cout << "\nPress 1 to review the order of execution in which the code for each process has been executed : ";
	cin >> choice;
	if (choice == 1)
	{
		cout << "\nThe order of execution in which the code for each process has been executed :-\n";
		for (list<string>::iterator itr_string = execution_order.begin(); itr_string != execution_order.end(); itr_string++)
		{
			cout << (*itr_string);
			cout << endl;
			cin.ignore();
		}
	}
	cin.ignore();
	system("cls");
	int total_waiting_time = 0;
	int total_completion_time = 0;
	cout << "\nOptimization criteria :\n\n";
	cout << "PROCESS_ID\tBurst_time\tCompletion_time\tTurnaround_time\tWaiting_time\n";
	for (int loop = 0; loop < ::Process_count; loop++)
	{
		cout << attr[loop].pid << "\t\t" << attr[loop].Burst_Time;
		cout << "\t\t" << attr[loop].Completion_Time;
		attr[loop].Turnaround_Time = attr[loop].Completion_Time - attr[loop].Arrival_Time;
		cout << "\t\t" << attr[loop].Turnaround_Time;
		//attr[loop].Waiting_Time = attr[loop].Start_Time - attr[loop].Arrival_Time;
		attr[loop].Waiting_Time = attr[loop].Completion_Time - (attr[loop].Arrival_Time + attr[loop].Burst_Time);
		cout << "\t\t" << attr[loop].Waiting_Time << endl;
		total_waiting_time += attr[loop].Waiting_Time;
		total_completion_time += attr[loop].Completion_Time;
	}
	cout << "\nTotal time : " << total_time << endl;
	cout << "Average waiting time : " << (total_waiting_time / ::Process_count) << endl;
	cout << "Average Completion time : " << (total_completion_time / ::Process_count) << endl;
	cin.ignore();

}



//SHORTEST JOB FIRST NON PREEMPTIVE 
void SJF_np()
{
	list<PROCESS> ready_queue;
	list<PROCESS> process_queue;
	queue<PROCESS> exit_queue;
	list<PROCESS> blocked_queue;

	PROCESS running_process;
	PROCESS dup_running_process;
	list<string> execution_order;
	ostringstream buffer;
	attributes* attr;
	attr = new attributes[::Process_count];

	int total_time = 0;
	int time2 = 0;

	//SORT ON THE BASIS OF ARRIVAL TIME
	for (int i = 0; i < ::Process_count - 1; i++)
	{
		for (int j = 0; j < ::Process_count - 1; j++)
		{
			if (Proccess_datum[j].arrival_time>Proccess_datum[j + 1].arrival_time)
			{
				PROCESS temp = Proccess_datum[j];
				Proccess_datum[j] = Proccess_datum[j + 1];
				Proccess_datum[j + 1] = temp;
			}
		}
	}
	for (int loop = 0; loop < ::Process_count; loop++)
	{
		process_queue.push_back(Proccess_datum[loop]);
		attr[loop].Arrival_Time = Proccess_datum[loop].arrival_time;
		attr[loop].pid = Proccess_datum[loop].pid;
		attr[loop].Burst_Time = Proccess_datum[loop].burst_time;
	}

	while (!ready_queue.empty() || !blocked_queue.empty() || !process_queue.empty())
	{
		time2++;
		for (list<PROCESS>::iterator itr = process_queue.begin(); itr != process_queue.end(); itr++)
		{
			if ((*itr).arrival_time <= total_time)
			{
				ready_queue.push_back((*itr));
				//DELETE THAT INSTANCE FROM THE process_queue !!
				list<PROCESS>::iterator dItr = process_queue.begin();
				while (dItr != process_queue.end())
				{
					if (dItr->pid == (*itr).pid)
					{
						process_queue.erase(dItr++);
						itr = process_queue.begin();
						break;
					}
					else
					{
						dItr++;
					}
				}
			}
			if (process_queue.size() == 0)
			{
				break;
			}
		}
		if (ready_queue.empty() && blocked_queue.empty() && !process_queue.empty())
		{
			total_time += (process_queue.front().arrival_time - total_time);
			time2 += (process_queue.front().arrival_time - total_time);
			ready_queue.push_back(process_queue.front());

			buffer << "Ready queue is empty unblocking from process queue Process : " << process_queue.front().pid;
			execution_order.push_back(buffer.str().c_str());
			buffer.str("");
			buffer.clear();

			process_queue.pop_front();
		}
		if (ready_queue.empty() && !blocked_queue.empty())
		{
			total_time += 2;
			time2 += 2;
			blocked_queue.front().q_code.pop();
			ready_queue.push_back(blocked_queue.front());

			buffer << "Ready queue is empty unblocking from blocked queue Process : " << blocked_queue.front().pid;
			execution_order.push_back(buffer.str().c_str());
			buffer.str("");
			buffer.clear();

			blocked_queue.pop_front();
		}
		//SORT READY QUEUE ACCORDING TO SHORTEST BURST TIME
		ready_queue.sort(PROCESS_SORT);

		running_process = ready_queue.front();
		dup_running_process = running_process;

		for (int loop = 0; loop < ::Process_count; loop++)
		{
			if (attr[loop].pid == dup_running_process.pid && attr[loop].Start_Time == -1)
			{
				attr[loop].Start_Time = time2 - 1;
			}
		}
		ready_queue.pop_front();

		while (!running_process.q_code.empty())
		{
			total_time++;
			time2++;
			if (!blocked_queue.empty())
			{
				if (blocked_queue.front().blocked_time <= 0)
				{
					buffer << "I/O done. blocking instruction : " << blocked_queue.front().q_code.front() << " executed !! Process " << blocked_queue.front().pid << " unblocked and moved back to ready queue from blocked queue.";
					execution_order.push_back(buffer.str().c_str());
					buffer.str("");
					buffer.clear();
					blocked_queue.front().q_code.pop();
					ready_queue.push_back(blocked_queue.front());
					blocked_queue.pop_front();
				}
				if (!blocked_queue.empty())
				{
					for (list<PROCESS>::iterator itr = blocked_queue.begin(); itr != blocked_queue.end(); itr++)
					{
						(*itr).blocked_time--;
					}
				}
			}
			int instype = typeoins(running_process.q_code.front());
			if (instype == 2 || instype == 1)
			{
				buffer << "Instrution : " << running_process.q_code.front() << " for Process : " << running_process.pid << " EXECUTED";
				execution_order.push_back(buffer.str().c_str());
				buffer.str("");
				buffer.clear();
				running_process.q_code.pop();
			}
			else if (instype == 3)
			{
				buffer << "BLOCKING Instrution : " << running_process.q_code.front() << " for Process : " << running_process.pid;
				execution_order.push_back(buffer.str().c_str());
				buffer.str("");
				buffer.clear();

				running_process.blocked_time = 3;
				blocked_queue.push_back(running_process);
				break;

			}
			else if (instype == 4)
			{
				cout << "\nBREAKPOINT !!\n\n";
				running_process.q_code.pop();
				cin.ignore();

				list<PROCESS> dup_ready_queue = ready_queue;
				queue<PROCESS> dup_exit_queue = exit_queue;

				cout << "\nProcesses Yet to arrive (in the new queue) : ";
				for (list<PROCESS>::iterator itr = process_queue.begin(); itr != process_queue.end(); itr++)
				{
					cout << (*itr).pid << " ";
				}
				cout << "\n\n";


				cout << "\nProcesses currently running : " << running_process.pid;
				cout << "\nProcesses in Ready state(queue) : ";
				while (!dup_ready_queue.empty())
				{
					cout << dup_ready_queue.front().pid << " ";
					dup_ready_queue.pop_front();
				}
				cout << "\nProcesses in Exit state(queue) : ";
				while (!dup_exit_queue.empty())
				{
					cout << dup_exit_queue.front().pid << " ";
					dup_exit_queue.pop();
				}
				cout << "\nProcesses in Blocked(waiting) state(queue) : ";
				for (list<PROCESS>::iterator itr = blocked_queue.begin(); itr != blocked_queue.end(); itr++)
				{
					cout << (*itr).pid << " ";
				}
				cin.ignore();
				cout << "\nThe order of execution in which the code for each process has been executed till now :-\n";
				for (list<string>::iterator itr_string = execution_order.begin(); itr_string != execution_order.end(); itr_string++)
				{
					cout << (*itr_string);
					cout << endl;
					cin.ignore();
				}
			}
		}
		if (running_process.q_code.empty())
		{
			buffer << "PROCESS : " << dup_running_process.pid << " COMLETED ITS EXECUTION. PUSHED INTO EXIT QUEUE";
			execution_order.push_back(buffer.str().c_str());
			buffer.str("");
			buffer.clear();
			for (int loop = 0; loop < ::Process_count; loop++)
			{
				if (attr[loop].pid == dup_running_process.pid)
				{
					attr[loop].Completion_Time = total_time;
				}
			}
			exit_queue.push(dup_running_process);
		}
		if (ready_queue.empty() && blocked_queue.empty())
		{
			break;
		}
	}
	cout << "\nAll processes have been succesfully executed." << endl;
	cin.ignore();
	system("cls");
	int choice;
	cout << "\nPress 1 to review the order of execution in which the code for each process has been executed : ";
	cin >> choice;
	if (choice == 1)
	{
		cout << "\nThe order of execution in which the code for each process has been executed :-\n";
		for (list<string>::iterator itr_string = execution_order.begin(); itr_string != execution_order.end(); itr_string++)
		{
			cout << (*itr_string);
			cout << endl;
			cin.ignore();
		}
	}
	cin.ignore();
	system("cls");
	int total_waiting_time = 0;
	int total_completion_time = 0;
	cout << "\nOptimization criteria :\n\n";
	cout << "PROCESS_ID\tBurst_time\tCompletion_time\tTurnaround_time\tWaiting_time\n";
	for (int loop = 0; loop < ::Process_count; loop++)
	{
		cout << attr[loop].pid << "\t\t" << attr[loop].Burst_Time;
		cout << "\t\t" << attr[loop].Completion_Time;
		attr[loop].Turnaround_Time = attr[loop].Completion_Time - attr[loop].Arrival_Time;
		cout << "\t\t" << attr[loop].Turnaround_Time;
		//attr[loop].Waiting_Time = attr[loop].Start_Time - attr[loop].Arrival_Time;
		attr[loop].Waiting_Time = attr[loop].Completion_Time - (attr[loop].Arrival_Time + attr[loop].Burst_Time);
		cout << "\t\t" << attr[loop].Waiting_Time << endl;
		total_waiting_time += attr[loop].Waiting_Time;
		total_completion_time += attr[loop].Completion_Time;
	}
	cout << "\nTotal time : " << total_time << endl;
	cout << "Average waiting time : " << (total_waiting_time / ::Process_count) << endl;
	cout << "Average Completion time : " << (total_completion_time / ::Process_count) << endl;
	cin.ignore();

}



//SHORTEST JOB FIRST PREEMPTIVE (SHORTEST REMAINING TIME FIRST)
void SRTF()
{
	list<PROCESS> ready_queue;
	list<PROCESS> process_queue;
	queue<PROCESS> exit_queue;
	list<PROCESS> blocked_queue;

	PROCESS running_process;
	PROCESS dup_running_process;
	list<string> execution_order;
	ostringstream buffer;
	attributes* attr;
	attr = new attributes[::Process_count];

	int total_time = 0;
	int time2 = 0;

	//SORT ON THE BASIS OF ARRIVAL TIME
	for (int i = 0; i < ::Process_count - 1; i++)
	{
		for (int j = 0; j < ::Process_count - 1; j++)
		{
			if (Proccess_datum[j].arrival_time>Proccess_datum[j + 1].arrival_time)
			{
				PROCESS temp = Proccess_datum[j];
				Proccess_datum[j] = Proccess_datum[j + 1];
				Proccess_datum[j + 1] = temp;
			}
		}
	}
	for (int loop = 0; loop < ::Process_count; loop++)
	{
		process_queue.push_back(Proccess_datum[loop]);
		attr[loop].Arrival_Time = Proccess_datum[loop].arrival_time;
		attr[loop].pid = Proccess_datum[loop].pid;
		attr[loop].Burst_Time = Proccess_datum[loop].burst_time;
	}
	while (!ready_queue.empty() || !blocked_queue.empty() || !process_queue.empty())
	{
		time2++;
		for (list<PROCESS>::iterator itr = process_queue.begin(); itr != process_queue.end(); itr++)
		{
			if ((*itr).arrival_time <= total_time)
			{
				ready_queue.push_back((*itr));
				//DELETE THAT INSTANCE FROM THE process_queue !!
				list<PROCESS>::iterator dItr = process_queue.begin();
				while (dItr != process_queue.end())
				{
					if (dItr->pid == (*itr).pid)
					{
						process_queue.erase(dItr++);
						itr = process_queue.begin();
						break;
					}
					else
					{
						dItr++;
					}
				}
			}
			if (process_queue.size() == 0)
			{
				break;
			}
		}
		if (ready_queue.empty() && blocked_queue.empty() && !process_queue.empty())
		{
			total_time += (process_queue.front().arrival_time - total_time);
			time2 += (process_queue.front().arrival_time - total_time);
			ready_queue.push_back(process_queue.front());

			buffer << "Ready queue is empty unblocking from process queue Process : " << process_queue.front().pid;
			execution_order.push_back(buffer.str().c_str());
			buffer.str("");
			buffer.clear();

			process_queue.pop_front();
		}
		if (ready_queue.empty() && !blocked_queue.empty())
		{
			total_time += 2;
			time2 += 2;
			blocked_queue.front().q_code.pop();
			blocked_queue.front().burst_time -= 2;
			ready_queue.push_back(blocked_queue.front());

			buffer << "Ready queue is empty unblocking from blocked queue Process : " << blocked_queue.front().pid;
			execution_order.push_back(buffer.str().c_str());
			buffer.str("");
			buffer.clear();

			blocked_queue.pop_front();
		}
		//SORT READY QUEUE ACCORDING TO SHORTEST BURST TIME
		ready_queue.sort(PROCESS_SORT);

		running_process = ready_queue.front();
		dup_running_process = running_process;

		for (int loop = 0; loop < ::Process_count; loop++)
		{
			if (attr[loop].pid == dup_running_process.pid && attr[loop].Start_Time == -1)
			{
				attr[loop].Start_Time = time2 - 1;
			}
		}
		ready_queue.pop_front();
		while (!running_process.q_code.empty())
		{
			total_time++;
			time2++;
			//PREEMPTION 

			if (process_queue.size() != 0 && total_time>1)
			{
				for (list<PROCESS>::iterator itr = process_queue.begin(); itr != process_queue.end(); itr++)
				{
					if ((*itr).arrival_time <= total_time)
					{
						ready_queue.push_back((*itr));
						//DELETE THAT INSTANCE FROM THE process_queue !!
						list<PROCESS>::iterator dItr = process_queue.begin();
						while (dItr != process_queue.end())
						{
							if (dItr->pid == (*itr).pid)
							{
								process_queue.erase(dItr++);
								itr = process_queue.begin();
								break;
							}
							else
							{
								dItr++;
							}
						}
					}
					if (process_queue.size() == 0)
					{
						break;
					}
				}
			}
			ready_queue.sort(PROCESS_SORT);
			if (ready_queue.size() != 0)
			{
				if (running_process.burst_time > ready_queue.front().burst_time)
				{
					int temp_pid = running_process.pid;
					int temp_burst = running_process.burst_time;
					ready_queue.push_back(running_process);
					running_process = ready_queue.front();
					dup_running_process = running_process;
					buffer << "Preempt occured ! Context switched from process : " << temp_pid << "( " << temp_burst << " )" << " TO -->  " << running_process.pid << "( " << running_process.burst_time << " )";
					execution_order.push_back(buffer.str().c_str());
					buffer.str("");
					buffer.clear();
					ready_queue.pop_front();
				}
			}

			for (int loop = 0; loop < ::Process_count; loop++)
			{
				if (attr[loop].pid == dup_running_process.pid && attr[loop].Start_Time == -1)
				{
					attr[loop].Start_Time = time2 - 1;
				}
			}


			if (!blocked_queue.empty())
			{
				if (blocked_queue.front().blocked_time <= 0)
				{
					buffer << "I/O done. blocking instruction : " << blocked_queue.front().q_code.front() << " executed !! Process " << blocked_queue.front().pid << " unblocked and moved back to ready queue from blocked queue.";
					execution_order.push_back(buffer.str().c_str());
					buffer.str("");
					buffer.clear();
					blocked_queue.front().q_code.pop();
					ready_queue.push_back(blocked_queue.front());
					blocked_queue.pop_front();
				}
				if (!blocked_queue.empty())
				{
					for (list<PROCESS>::iterator itr = blocked_queue.begin(); itr != blocked_queue.end(); itr++)
					{
						(*itr).blocked_time--;
					}
				}
			}
			int instype = typeoins(running_process.q_code.front());
			if (instype == 2 || instype == 1)
			{
				buffer << "Instrution : " << running_process.q_code.front() << " for Process : " << running_process.pid << " EXECUTED";
				execution_order.push_back(buffer.str().c_str());
				buffer.str("");
				buffer.clear();
				running_process.burst_time -= instype;
				running_process.q_code.pop();
			}
			else if (instype == 3)
			{
				buffer << "BLOCKING Instrution : " << running_process.q_code.front() << " for Process : " << running_process.pid;
				execution_order.push_back(buffer.str().c_str());
				buffer.str("");
				buffer.clear();

				running_process.blocked_time = 3;
				blocked_queue.push_back(running_process);
				break;

			}
			else if (instype == 4)
			{
				cout << "\nBREAKPOINT !!\n\n";
				running_process.q_code.pop();
				cin.ignore();

				list<PROCESS> dup_ready_queue = ready_queue;
				queue<PROCESS> dup_exit_queue = exit_queue;

				cout << "\nProcesses Yet to arrive (in the new queue) : ";
				for (list<PROCESS>::iterator itr = process_queue.begin(); itr != process_queue.end(); itr++)
				{
					cout << (*itr).pid << " ";
				}
				cout << "\n\n";
				cout << "\nProcesses currently running : " << running_process.pid;
				cout << "\nProcesses in Ready state(queue) : ";
				while (!dup_ready_queue.empty())
				{
					cout << dup_ready_queue.front().pid << " ";
					dup_ready_queue.pop_front();
				}
				cout << "\nProcesses in Exit state(queue) : ";
				while (!dup_exit_queue.empty())
				{
					cout << dup_exit_queue.front().pid << " ";
					dup_exit_queue.pop();
				}
				cout << "\nProcesses in Blocked(waiting) state(queue) : ";
				for (list<PROCESS>::iterator itr = blocked_queue.begin(); itr != blocked_queue.end(); itr++)
				{
					cout << (*itr).pid << " ";
				}
				cin.ignore();
				cout << "\nThe order of execution in which the code for each process has been executed till now :-\n";
				for (list<string>::iterator itr_string = execution_order.begin(); itr_string != execution_order.end(); itr_string++)
				{
					cout << (*itr_string);
					cout << endl;
					cin.ignore();
				}
			}
		}
		if (running_process.q_code.empty())
		{
			buffer << "PROCESS : " << dup_running_process.pid << " COMLETED ITS EXECUTION. PUSHED INTO EXIT QUEUE";
			execution_order.push_back(buffer.str().c_str());
			buffer.str("");
			buffer.clear();
			for (int loop = 0; loop < ::Process_count; loop++)
			{
				if (attr[loop].pid == dup_running_process.pid)
				{
					attr[loop].Completion_Time = total_time;
				}
			}
			exit_queue.push(dup_running_process);
		}
		if (ready_queue.empty() && blocked_queue.empty())
		{
			break;
		}
	}
	cout << "\nAll processes have been succesfully executed." << endl;
	cin.ignore();
	system("cls");
	int choice;
	cout << "\nPress 1 to review the order of execution in which the code for each process has been executed : ";
	cin >> choice;
	if (choice == 1)
	{
		cout << "\nThe order of execution in which the code for each process has been executed :-\n";
		for (list<string>::iterator itr_string = execution_order.begin(); itr_string != execution_order.end(); itr_string++)
		{
			cout << (*itr_string);
			cout << endl;
			cin.ignore();
		}
	}
	cin.ignore();
	system("cls");
	int total_waiting_time = 0;
	int total_completion_time = 0;
	cout << "\nOptimization criteria :\n\n";
	cout << "PROCESS_ID\tBurst_time\tCompletion_time\tTurnaround_time\tWaiting_time\n";
	for (int loop = 0; loop < ::Process_count; loop++)
	{
		cout << attr[loop].pid << "\t\t" << attr[loop].Burst_Time;
		cout << "\t\t" << attr[loop].Completion_Time;
		attr[loop].Turnaround_Time = attr[loop].Completion_Time - attr[loop].Arrival_Time;
		cout << "\t\t" << attr[loop].Turnaround_Time;
		//attr[loop].Waiting_Time = attr[loop].Start_Time - attr[loop].Arrival_Time;
		attr[loop].Waiting_Time = attr[loop].Completion_Time - (attr[loop].Arrival_Time + attr[loop].Burst_Time);
		cout << "\t\t" << attr[loop].Waiting_Time << endl;
		total_waiting_time += attr[loop].Waiting_Time;
		total_completion_time += attr[loop].Completion_Time;
	}
	cout << "\nTotal time : " << total_time << endl;
	cout << "Average waiting time : " << (total_waiting_time / ::Process_count) << endl;
	cout << "Average Completion time : " << (total_completion_time / ::Process_count) << endl;
	cin.ignore();

}





void PS_np()
{
	list<PROCESS> ready_queue;
	list<PROCESS> process_queue;
	queue<PROCESS> exit_queue;
	list<PROCESS> blocked_queue;

	PROCESS running_process;
	PROCESS dup_running_process;
	list<string> execution_order;
	ostringstream buffer;
	attributes* attr;
	attr = new attributes[::Process_count];

	int total_time = 0;
	int time2 = 0;

	//SORT ON THE BASIS OF ARRIVAL TIME
	for (int i = 0; i < ::Process_count - 1; i++)
	{
		for (int j = 0; j < ::Process_count - 1; j++)
		{
			if (Proccess_datum[j].arrival_time>Proccess_datum[j + 1].arrival_time)
			{
				PROCESS temp = Proccess_datum[j];
				Proccess_datum[j] = Proccess_datum[j + 1];
				Proccess_datum[j + 1] = temp;
			}
		}
	}
	for (int loop = 0; loop < ::Process_count; loop++)
	{
		process_queue.push_back(Proccess_datum[loop]);
		attr[loop].Arrival_Time = Proccess_datum[loop].arrival_time;
		attr[loop].pid = Proccess_datum[loop].pid;
		attr[loop].Burst_Time = Proccess_datum[loop].burst_time;
	}


	while (!ready_queue.empty() || !blocked_queue.empty() || !process_queue.empty())
	{
		time2++;
		for (list<PROCESS>::iterator itr = process_queue.begin(); itr != process_queue.end(); itr++)
		{
			if ((*itr).arrival_time <= total_time)
			{
				ready_queue.push_back((*itr));
				//DELETE THAT INSTANCE FROM THE process_queue !!
				list<PROCESS>::iterator dItr = process_queue.begin();
				while (dItr != process_queue.end())
				{
					if (dItr->pid == (*itr).pid)
					{
						process_queue.erase(dItr++);
						itr = process_queue.begin();
						break;
					}
					else
					{
						dItr++;
					}
				}
			}
			if (process_queue.size() == 0)
			{
				break;
			}
		}
		if (ready_queue.empty() && blocked_queue.empty() && !process_queue.empty())
		{
			total_time += (process_queue.front().arrival_time - total_time);
			time2 += (process_queue.front().arrival_time - total_time);
			ready_queue.push_back(process_queue.front());

			buffer << "Ready queue is empty unblocking from process queue Process : " << process_queue.front().pid;
			execution_order.push_back(buffer.str().c_str());
			buffer.str("");
			buffer.clear();

			process_queue.pop_front();
		}
		if (ready_queue.empty() && !blocked_queue.empty())
		{
			total_time += 2;
			time2 += 2;
			blocked_queue.front().q_code.pop();
			ready_queue.push_back(blocked_queue.front());

			buffer << "Ready queue is empty unblocking from blocked queue Process : " << blocked_queue.front().pid;
			execution_order.push_back(buffer.str().c_str());
			buffer.str("");
			buffer.clear();

			blocked_queue.pop_front();
		}
		//SORT READY QUEUE ACCORDING TO SHORTEST BURST TIME
		ready_queue.sort(PROCESS_SORT_PS);

		running_process = ready_queue.front();
		dup_running_process = running_process;

		for (int loop = 0; loop < ::Process_count; loop++)
		{
			if (attr[loop].pid == dup_running_process.pid && attr[loop].Start_Time == -1)
			{
				attr[loop].Start_Time = time2 - 1;
			}
		}
		ready_queue.pop_front();

		while (!running_process.q_code.empty())
		{
			total_time++;
			time2++;
			if (!blocked_queue.empty())
			{
				if (blocked_queue.front().blocked_time <= 0)
				{
					buffer << "I/O done. blocking instruction : " << blocked_queue.front().q_code.front() << " executed !! Process " << blocked_queue.front().pid << " unblocked and moved back to ready queue from blocked queue.";
					execution_order.push_back(buffer.str().c_str());
					buffer.str("");
					buffer.clear();
					blocked_queue.front().q_code.pop();
					ready_queue.push_back(blocked_queue.front());
					blocked_queue.pop_front();
				}
				if (!blocked_queue.empty())
				{
					for (list<PROCESS>::iterator itr = blocked_queue.begin(); itr != blocked_queue.end(); itr++)
					{
						(*itr).blocked_time--;
					}
				}
			}
			int instype = typeoins(running_process.q_code.front());
			if (instype == 2 || instype == 1)
			{
				buffer << "Instrution : " << running_process.q_code.front() << " for Process : " << running_process.pid << " EXECUTED";
				execution_order.push_back(buffer.str().c_str());
				buffer.str("");
				buffer.clear();
				running_process.q_code.pop();
			}
			else if (instype == 3)
			{
				buffer << "BLOCKING Instrution : " << running_process.q_code.front() << " for Process : " << running_process.pid;
				execution_order.push_back(buffer.str().c_str());
				buffer.str("");
				buffer.clear();

				running_process.blocked_time = 3;
				blocked_queue.push_back(running_process);
				break;

			}
			else if (instype == 4)
			{
				cout << "\nBREAKPOINT !!\n\n";
				running_process.q_code.pop();
				cin.ignore();

				list<PROCESS> dup_ready_queue = ready_queue;
				queue<PROCESS> dup_exit_queue = exit_queue;
				
				cout << "\nProcesses Yet to arrive (in the new queue) : ";
				for (list<PROCESS>::iterator itr = process_queue.begin(); itr != process_queue.end(); itr++)
				{
					cout << (*itr).pid << " ";
				}
				cout << "\n\n";

				cout << "\nProcesses currently running : " << running_process.pid;
				cout << "\nProcesses in Ready state(queue) : ";
				while (!dup_ready_queue.empty())
				{
					cout << dup_ready_queue.front().pid << " ";
					dup_ready_queue.pop_front();
				}
				cout << "\nProcesses in Exit state(queue) : ";
				while (!dup_exit_queue.empty())
				{
					cout << dup_exit_queue.front().pid << " ";
					dup_exit_queue.pop();
				}
				cout << "\nProcesses in Blocked(waiting) state(queue) : ";
				for (list<PROCESS>::iterator itr = blocked_queue.begin(); itr != blocked_queue.end(); itr++)
				{
					cout << (*itr).pid << " ";
				}
				cin.ignore();
				cout << "\nThe order of execution in which the code for each process has been executed till now :-\n";
				for (list<string>::iterator itr_string = execution_order.begin(); itr_string != execution_order.end(); itr_string++)
				{
					cout << (*itr_string);
					cout << endl;
					cin.ignore();
				}
			}
		}
		if (running_process.q_code.empty())
		{
			buffer << "PROCESS : " << dup_running_process.pid << " COMLETED ITS EXECUTION. PUSHED INTO EXIT QUEUE";
			execution_order.push_back(buffer.str().c_str());
			buffer.str("");
			buffer.clear();
			for (int loop = 0; loop < ::Process_count; loop++)
			{
				if (attr[loop].pid == dup_running_process.pid)
				{
					attr[loop].Completion_Time = total_time;
				}
			}
			exit_queue.push(dup_running_process);
		}
		if (ready_queue.empty() && blocked_queue.empty())
		{
			break;
		}
	}
	cout << "\nAll processes have been succesfully executed." << endl;
	cin.ignore();
	system("cls");
	int choice;
	cout << "\nPress 1 to review the order of execution in which the code for each process has been executed : ";
	cin >> choice;
	if (choice == 1)
	{
		cout << "\nThe order of execution in which the code for each process has been executed :-\n";
		for (list<string>::iterator itr_string = execution_order.begin(); itr_string != execution_order.end(); itr_string++)
		{
			cout << (*itr_string);
			cout << endl;
			cin.ignore();
		}
	}
	cin.ignore();
	system("cls");
	int total_waiting_time = 0;
	int total_completion_time = 0;
	cout << "\nOptimization criteria :\n\n";
	cout << "PROCESS_ID\tBurst_time\tCompletion_time\tTurnaround_time\tWaiting_time\n";
	for (int loop = 0; loop < ::Process_count; loop++)
	{
		cout << attr[loop].pid << "\t\t" << attr[loop].Burst_Time;
		cout << "\t\t" << attr[loop].Completion_Time;
		attr[loop].Turnaround_Time = attr[loop].Completion_Time - attr[loop].Arrival_Time;
		cout << "\t\t" << attr[loop].Turnaround_Time;
		//attr[loop].Waiting_Time = attr[loop].Start_Time - attr[loop].Arrival_Time;
		attr[loop].Waiting_Time = attr[loop].Completion_Time - (attr[loop].Arrival_Time + attr[loop].Burst_Time);
		cout << "\t\t" << attr[loop].Waiting_Time << endl;
		total_waiting_time += attr[loop].Waiting_Time;
		total_completion_time += attr[loop].Completion_Time;
	}
	cout << "\nTotal time : " << total_time << endl;
	cout << "Average waiting time : " << (total_waiting_time / ::Process_count) << endl;
	cout << "Average Completion time : " << (total_completion_time / ::Process_count) << endl;
	cin.ignore();

}



void PS_p()
{
	cin.ignore();
	list<PROCESS> ready_queue;
	list<PROCESS> process_queue;
	queue<PROCESS> exit_queue;
	list<PROCESS> blocked_queue;

	PROCESS running_process;
	PROCESS dup_running_process;
	list<string> execution_order;
	ostringstream buffer;
	attributes* attr;
	attr = new attributes[::Process_count];

	int total_time = 0;
	int time2 = 0;

	//SORT ON THE BASIS OF ARRIVAL TIME
	for (int i = 0; i < ::Process_count - 1; i++)
	{
		for (int j = 0; j < ::Process_count - 1; j++)
		{
			if (Proccess_datum[j].arrival_time>Proccess_datum[j + 1].arrival_time)
			{
				PROCESS temp = Proccess_datum[j];
				Proccess_datum[j] = Proccess_datum[j + 1];
				Proccess_datum[j + 1] = temp;
			}
		}
	}
	for (int loop = 0; loop < ::Process_count; loop++)
	{
		process_queue.push_back(Proccess_datum[loop]);
		attr[loop].Arrival_Time = Proccess_datum[loop].arrival_time;
		attr[loop].pid = Proccess_datum[loop].pid;
		attr[loop].Burst_Time = Proccess_datum[loop].burst_time;
	}
	while (!ready_queue.empty() || !blocked_queue.empty() || !process_queue.empty())
	{
		time2++;
		for (list<PROCESS>::iterator itr = process_queue.begin(); itr != process_queue.end(); itr++)
		{
			if ((*itr).arrival_time <= total_time)
			{
				ready_queue.push_back((*itr));
				//DELETE THAT INSTANCE FROM THE process_queue !!
				list<PROCESS>::iterator dItr = process_queue.begin();
				while (dItr != process_queue.end())
				{
					if (dItr->pid == (*itr).pid)
					{
						process_queue.erase(dItr++);
						itr = process_queue.begin();
						break;
					}
					else
					{
						dItr++;
					}
				}
			}
			if (process_queue.size() == 0)
			{
				break;
			}
		}
		if (ready_queue.empty() && blocked_queue.empty() && !process_queue.empty())
		{
			total_time += (process_queue.front().arrival_time - total_time);
			time2 += (process_queue.front().arrival_time - total_time);
			ready_queue.push_back(process_queue.front());

			buffer << "Ready queue is empty unblocking from process queue Process : " << process_queue.front().pid;
			execution_order.push_back(buffer.str().c_str());
			buffer.str("");
			buffer.clear();

			process_queue.pop_front();
		}
		if (ready_queue.empty() && !blocked_queue.empty())
		{
			total_time += 2;
			time2 += 2;
			blocked_queue.front().q_code.pop();
			blocked_queue.front().burst_time -= 2;
			ready_queue.push_back(blocked_queue.front());

			buffer << "Ready queue is empty unblocking from blocked queue Process : " << blocked_queue.front().pid;
			execution_order.push_back(buffer.str().c_str());
			buffer.str("");
			buffer.clear();

			blocked_queue.pop_front();
		}

		ready_queue.sort(PROCESS_SORT_PS);
		
		running_process = ready_queue.front();
		dup_running_process = running_process;

		for (int loop = 0; loop < ::Process_count; loop++)
		{
			if (attr[loop].pid == dup_running_process.pid && attr[loop].Start_Time == -1)
			{
				attr[loop].Start_Time = time2 - 1;
			}
		}
		ready_queue.pop_front();

		while (!running_process.q_code.empty())
		{
			total_time++;
			time2++;
			//PREEMPTION 

			if (process_queue.size() != 0 && total_time>1)
			{
				for (list<PROCESS>::iterator itr = process_queue.begin(); itr != process_queue.end(); itr++)
				{
					if ((*itr).arrival_time <= total_time)
					{
						ready_queue.push_back((*itr));
						//DELETE THAT INSTANCE FROM THE process_queue !!
						list<PROCESS>::iterator dItr = process_queue.begin();
						while (dItr != process_queue.end())
						{
							if (dItr->pid == (*itr).pid)
							{
								process_queue.erase(dItr++);
								itr = process_queue.begin();
								break;
							}
							else
							{
								dItr++;
							}
						}
					}
					if (process_queue.size() == 0)
					{
						break;
					}
				}
			}
			ready_queue.sort(PROCESS_SORT_PS);
		
			if (ready_queue.size() != 0)
			{
				if (running_process.priority < ready_queue.front().priority)
				{
					int temp_pid = running_process.pid;
					int temp_priority = running_process.priority;
					ready_queue.push_back(running_process);
					running_process = ready_queue.front();
					dup_running_process = running_process;
					buffer << "Preempt occured ! Context switched from process : " << temp_pid << "( " << temp_priority << " )" << " TO -->  " << running_process.pid << "( " << running_process.priority << " )\n3=High, 2=Medium, 3=Low";
					execution_order.push_back(buffer.str().c_str());
					buffer.str("");
					buffer.clear();
					ready_queue.pop_front();
				}
			}
			for (int loop = 0; loop < ::Process_count; loop++)
			{
				if (attr[loop].pid == dup_running_process.pid && attr[loop].Start_Time == -1)
				{
					attr[loop].Start_Time = time2 - 1;
				}
			}


			if (!blocked_queue.empty())
			{
				if (blocked_queue.front().blocked_time <= 0)
				{
					buffer << "I/O done. blocking instruction : " << blocked_queue.front().q_code.front() << " executed !! Process " << blocked_queue.front().pid << " unblocked and moved back to ready queue from blocked queue.";
					execution_order.push_back(buffer.str().c_str());
					buffer.str("");
					buffer.clear();
					blocked_queue.front().q_code.pop();
					ready_queue.push_back(blocked_queue.front());
					blocked_queue.pop_front();
				}
				if (!blocked_queue.empty())
				{
					for (list<PROCESS>::iterator itr = blocked_queue.begin(); itr != blocked_queue.end(); itr++)
					{
						(*itr).blocked_time--;
					}
				}
			}
			int instype = typeoins(running_process.q_code.front());
			if (instype == 2 || instype == 1)
			{
				buffer << "Instrution : " << running_process.q_code.front() << " for Process : " << running_process.pid << " EXECUTED";
				execution_order.push_back(buffer.str().c_str());
				buffer.str("");
				buffer.clear();
				running_process.burst_time -= instype;
				running_process.q_code.pop();
			}
			else if (instype == 3)
			{
				buffer << "BLOCKING Instrution : " << running_process.q_code.front() << " for Process : " << running_process.pid;
				execution_order.push_back(buffer.str().c_str());
				buffer.str("");
				buffer.clear();

				running_process.blocked_time = 3;
				blocked_queue.push_back(running_process);
				break;

			}
			else if (instype == 4)
			{
				cout << "\nBREAKPOINT !!\n\n";
				running_process.q_code.pop();
				cin.ignore();

				list<PROCESS> dup_ready_queue = ready_queue;
				queue<PROCESS> dup_exit_queue = exit_queue;

				cout << "\nProcesses Yet to arrive (in the new queue) : ";
				for (list<PROCESS>::iterator itr = process_queue.begin(); itr != process_queue.end(); itr++)
				{
					cout << (*itr).pid << " ";
				}
				cout << "\n\n";

				cout << "\nProcesses currently running : " << running_process.pid;
				cout << "\nProcesses in Ready state(queue) : ";
				while (!dup_ready_queue.empty())
				{
					cout << dup_ready_queue.front().pid << " ";
					dup_ready_queue.pop_front();
				}
				cout << "\nProcesses in Exit state(queue) : ";
				while (!dup_exit_queue.empty())
				{
					cout << dup_exit_queue.front().pid << " ";
					dup_exit_queue.pop();
				}
				cout << "\nProcesses in Blocked(waiting) state(queue) : ";
				for (list<PROCESS>::iterator itr = blocked_queue.begin(); itr != blocked_queue.end(); itr++)
				{
					cout << (*itr).pid << " ";
				}
				cin.ignore();
				cout << "\nThe order of execution in which the code for each process has been executed till now :-\n";
				for (list<string>::iterator itr_string = execution_order.begin(); itr_string != execution_order.end(); itr_string++)
				{
					cout << (*itr_string);
					cout << endl;
					cin.ignore();
				}
			}
		}
		if (running_process.q_code.empty())
		{
			buffer << "PROCESS : " << dup_running_process.pid << " COMLETED ITS EXECUTION. PUSHED INTO EXIT QUEUE";
			execution_order.push_back(buffer.str().c_str());
			buffer.str("");
			buffer.clear();
			for (int loop = 0; loop < ::Process_count; loop++)
			{
				if (attr[loop].pid == dup_running_process.pid)
				{
					attr[loop].Completion_Time = total_time;
				}
			}
			exit_queue.push(dup_running_process);
		}
		if (ready_queue.empty() && blocked_queue.empty())
		{
			break;
		}
		}
		cout << "\nAll processes have been succesfully executed." << endl;
		cin.ignore();
		system("cls");
		int choice;
		cout << "\nPress 1 to review the order of execution in which the code for each process has been executed : ";
		cin >> choice;
		if (choice == 1)
		{
			cout << "\nThe order of execution in which the code for each process has been executed :-\n";
			for (list<string>::iterator itr_string = execution_order.begin(); itr_string != execution_order.end(); itr_string++)
			{
				cout << (*itr_string);
				cout << endl;
				cin.ignore();
			}
		}
		cin.ignore();
		system("cls");
		int total_waiting_time = 0;
		int total_completion_time = 0;
		cout << "\nOptimization criteria :\n\n";
		cout << "PROCESS_ID\tBurst_time\tCompletion_time\tTurnaround_time\tWaiting_time\n";
		for (int loop = 0; loop < ::Process_count; loop++)
		{
			cout << attr[loop].pid << "\t\t" << attr[loop].Burst_Time;
			cout << "\t\t" << attr[loop].Completion_Time;
			attr[loop].Turnaround_Time = attr[loop].Completion_Time - attr[loop].Arrival_Time;
			cout << "\t\t" << attr[loop].Turnaround_Time;
			//attr[loop].Waiting_Time = attr[loop].Start_Time - attr[loop].Arrival_Time;
			attr[loop].Waiting_Time = attr[loop].Completion_Time - (attr[loop].Arrival_Time + attr[loop].Burst_Time);
			cout << "\t\t" << attr[loop].Waiting_Time << endl;
			total_waiting_time += attr[loop].Waiting_Time;
			total_completion_time += attr[loop].Completion_Time;
		}
		cout << "\nTotal time : " << total_time << endl;
		cout << "Average waiting time : " << (total_waiting_time / ::Process_count) << endl;
		cout << "Average Completion time : " << (total_completion_time / ::Process_count) << endl;
		cin.ignore();

}



void main()
{
	ifstream myfile("Input\SJFP.xml");
	xml_document<> doc;
	vector<char> buffer((istreambuf_iterator<char>(myfile)), istreambuf_iterator<char>());
	buffer.push_back('\0');
	doc.parse<0>(&buffer[0]);
	xml_node<> *root_node = doc.first_node();
	SYSTEM_CFG = Parse_CFG(root_node);
	SYSTEM_CFG.getter_CFG();

	Process_count = atoi(root_node->first_node()->next_sibling()->first_attribute()->value());
	Proccess_datum = new PROCESS[::Process_count];
	cout << "Process_count : " << Process_count << endl;
	cin.ignore();
	Parse_PROCESS(root_node);


	if (SYSTEM_CFG.Policy.find("FCFS") != std::string::npos)
	{
		FCFS();
		cin.ignore();
	}
	else if (SYSTEM_CFG.Policy.find("RR") != std::string::npos)
	{
		Round_Robin();
		cin.ignore();
	}
	else if (SYSTEM_CFG.Policy.find("SJF") != std::string::npos)
	{
		if (SYSTEM_CFG.prmpt.find("No") != std::string::npos)
		{
			SJF_np();
			cin.ignore();
		}
		else if (SYSTEM_CFG.prmpt.find("Yes") != std::string::npos)
		{
			SRTF();
			cin.ignore();
		}
	}
	else if (SYSTEM_CFG.Policy.find("PS") != std::string::npos)
	{
		if (SYSTEM_CFG.prmpt.find("No") != std::string::npos)
		{
			PS_np();
			cin.ignore();
		}
		else if (SYSTEM_CFG.prmpt.find("Yes") != std::string::npos)
		{
			PS_p();
			cin.ignore();
		}
	}
	else
	{
		cout << "\nYou have entered a wrong scheduling policy. Kindly, revisit your xml file and insert PROPER data\n";
		exit(0);
	}
}