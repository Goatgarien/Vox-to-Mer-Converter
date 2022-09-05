#include <sstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <map>
#include <utility>

using namespace std;
using std::cout;

int main() {

	ifstream inputFile;
	string inputFileName = "";
	cout << "Input file directory (C:\\...\\...\\Chart.vox): ";
	cin >> inputFileName;
	cout << endl;
	inputFile.open(inputFileName);

	ofstream outputFile;
	string outputFileName = "";
	cout << "Desired output file name (will auto add .mer): ";
	cin >> outputFileName;
	cout << endl;
	outputFileName += ".mer";
	outputFile.open(outputFileName);

	string line;
	bool readBeat = false;
	bool readBPM = false;

	/*
	Timing (appears in the first column of some sections) takes the form of <measure>,<beat>,<tick>
	Ticks come at the rate of (4 / timesig_bottom) * 48 per beat. For example, a measure of 4/4 has 4 beats of 48 ticks each.
	Ticks are 0-indexed while measures and beats are 1-indexed.
	*/

	string MBT;
	int measure;
	int beat;
	int tick;

	map<int, pair<int, int>> TSmap;
	map<int, pair<double, bool>> BPMmap;

	while (getline(inputFile, line)) {
		int beatFound = 0;
		string temp = "";

		if (readBeat && line != "#END") {
			pair<int, int> TimeSig;

			for (int i = 0; i < (int)line.size(); i++) {
				if (line[i] != ',' && beatFound < 2) {
					temp += line[i];
				}
				else if (line[i] != '\t' && beatFound >= 2) {
					temp += line[i];
				}
				else {
					switch (beatFound) {
					case 0:
						temp.erase(0, min(temp.find_first_not_of('0'), temp.size() - 1));
						measure = stoi(temp);
						break;
					case 1:
						temp.erase(0, min(temp.find_first_not_of('0'), temp.size() - 1));
						beat = stoi(temp);
						break;
					case 2:
						temp.erase(0, min(temp.find_first_not_of('0'), temp.size() - 1));
						tick = stoi(temp);
						break;
					case 3:
						TimeSig.first = stoi(temp);
						break;
					default:
						break;
					}
					beatFound++;
					temp = "";
				}
			}
			TimeSig.second = stoi(temp);

			TSmap[measure] = TimeSig;
			cout << measure << " " << beat << " " << tick << " " << TimeSig.first << " " << TimeSig.second << endl;
		}

		if (readBPM && line != "#END") {
			pair<double, bool> BPMpair;

			for (int i = 0; i < (int)line.size(); i++) {
				if (line[i] != ',' && beatFound < 2) {
					temp += line[i];
				}
				else if (line[i] != '\t' && beatFound >= 2) {
					temp += line[i];
				}
				else {
					switch (beatFound) {
					case 0:
						temp.erase(0, min(temp.find_first_not_of('0'), temp.size() - 1));
						measure = stoi(temp);
						break;
					case 1:
						temp.erase(0, min(temp.find_first_not_of('0'), temp.size() - 1));
						beat = stoi(temp);
						break;
					case 2:
						temp.erase(0, min(temp.find_first_not_of('0'), temp.size() - 1));
						tick = stoi(temp);
						break;
					case 3:
						BPMpair.first = stod(temp);
						break;
					default:
						break;
					}
					beatFound++;
					temp = "";
				}
			}
			BPMpair.second = false;
			if (temp == "4-")
				BPMpair.second = true;

			BPMmap[measure] = BPMpair;
			cout << measure << " " << beat << " " << tick << " " << BPMpair.first << " " << BPMpair.second << endl;
		}

		if (line == "#BEAT INFO") {
			readBeat = true;
		}
		if (line == "#BPM INFO") {
			readBPM = true;
		}
		if (line == "#END") {
			readBeat = false;
			readBPM = false;
		}
	}

	outputFile << "#MUSIC_SCORE_ID 0" << endl
		<< "#MUSIC_SCORE_VERSION 0" << endl
		<< "#GAME_VERSION" << endl
		<< "#MUSIC_FILE_PATH " << outputFileName << endl
		<< "#OFFSET 0.000000" << endl
		<< "#MOVIEOFFSET 0.000000" << endl
		<< "#BODY" << endl;

	bool prevWasStop = false;
	int currMeasureTS = 1;
	int currMeasureBPM = 1;
	map<int, pair<int, int>>::iterator TSit = TSmap.begin();
	map<int, pair<double, bool>>::iterator BPMit = BPMmap.begin();
	bool endOfTSmap = false;
	bool endOfBPMmap = false;
	bool endOfBothMaps = false;

	if (TSit == TSmap.end())
		endOfTSmap = true;
	if (BPMit == BPMmap.end())
		endOfBPMmap = true;
	if (endOfTSmap && endOfBPMmap)
		endOfBothMaps = true;

	while (!endOfBothMaps) {
		if (!endOfTSmap) {
			if (TSit->first <= currMeasureBPM) {
				outputFile << std::right << std::fixed << std::setw(4) << (TSit->first - 1)
					<< std::right << std::fixed << std::setw(5) << 0
					<< std::right << std::fixed << std::setw(5) << 3
					<< std::right << std::fixed << std::setw(5) << TSit->second.first
					<< std::right << std::fixed << std::setw(5) << TSit->second.second
					<< endl;
				TSit++;
				if (TSit != TSmap.end()) {
					currMeasureTS = TSit->first;
				}
			}
		}
		if (!endOfBPMmap) {
			if (BPMit->first <= currMeasureTS) {
				if (!BPMit->second.second && !prevWasStop) {
					outputFile << std::right << std::fixed << std::setw(4) << (BPMit->first - 1)
						<< std::right << std::fixed << std::setw(5) << 0
						<< std::right << std::fixed << std::setw(5) << 2
						<< " " << std::right << std::fixed << std::setw(5) << BPMit->second.first
						<< endl;
				}
				else if (!BPMit->second.second && prevWasStop) {
					outputFile << std::right << std::fixed << std::setw(4) << (BPMit->first - 1)
						<< std::right << std::fixed << std::setw(5) << 0
						<< std::right << std::fixed << std::setw(5) << 10
						<< endl;
					prevWasStop = false;
				}
				else {
					outputFile << std::right << std::fixed << std::setw(4) << (BPMit->first - 1)
						<< std::right << std::fixed << std::setw(5) << 0
						<< std::right << std::fixed << std::setw(5) << 9
						<< endl;
					prevWasStop = true;
				}
				BPMit++;
				if (BPMit != BPMmap.end()) {
					currMeasureBPM = BPMit->first;
				}
			}
		}

		if (TSit == TSmap.end()) {
			endOfTSmap = true;
			currMeasureTS = 99999;
		}
		if (BPMit == BPMmap.end()) {
			endOfBPMmap = true;
			currMeasureBPM = 99999;
		}
		if (endOfTSmap && endOfBPMmap)
			endOfBothMaps = true;
	}


	inputFile.close();
	outputFile.close();
	return 0;
}