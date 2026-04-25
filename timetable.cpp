#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <climits>
using namespace std;

const int MAX = 100;
const int MAX_SLOTS = 20;

int n, rooms, teacherCount;
string subjects[MAX];
int students[MAX];
int conflict[MAX][MAX];
int slot[MAX];

string roomNames[MAX];
int roomCapacity[MAX];
string teachers[MAX];
string teacherRank[MAX];
int teacherAvailability[MAX][MAX_SLOTS];
int teacherUsage[MAX] = {0};

// ─────────────────────────── RANK HELPER ───────────────────────────
enum RankType { ASSISTANT_PROF, ASSOCIATE_PROF, PROFESSOR, INVALID_RANK };

RankType getRank(const string& rank) {
    if (rank.find("Assistant Professor") != string::npos) return ASSISTANT_PROF;
    if (rank.find("Associate Professor") != string::npos) return ASSOCIATE_PROF;
    if (rank.find("Professor") != string::npos) return PROFESSOR;
    return INVALID_RANK;
}

// ─────────────────────────── LOAD SUBJECTS ───────────────────────────
void loadSubjects() {
    ifstream file("Subject_Enrollment_Summary.csv");
    if (!file.is_open()) {
        cout << "Error: Subject_Enrollment_Summary.csv not found!\n";
        n = 0;
        return;
    }

    string line;
    getline(file, line);
    n = 0;

    while (getline(file, line) && n < MAX) {
        stringstream ss(line);
        string subj, enroll;
        if (getline(ss, subj, ',')) {
            subjects[n] = subj;
            students[n] = 30;
            if (getline(ss, enroll, ',')) {
                try { students[n] = stoi(enroll); } catch (...) {}
            }
            n++;
        }
    }

    cout << "Loaded " << n << " subjects.\n";
    file.close();
}

// ─────────────────────────── LOAD CONFLICT ───────────────────────────
void loadConflict() {
    ifstream file("Conflict_Matrix_150_Students.csv");
    if (!file.is_open()) {
        cout << "Error: Conflict_Matrix_150_Students.csv not found!\n";
        return;
    }

    memset(conflict, 0, sizeof(conflict));
    string line;
    getline(file, line);
    int i = 0;

    while (getline(file, line) && i < MAX) {
        stringstream ss(line);
        string val;
        getline(ss, val, ',');
        int j = 0;
        while (getline(ss, val, ',') && j < MAX) {
            try {
                if (!val.empty()) conflict[i][j] = stoi(val);
            } catch (...) {
                conflict[i][j] = 0;
            }
            j++;
        }
        i++;
    }

    cout << "Loaded conflict matrix (" << i << " rows).\n";
    file.close();
}

// ─────────────────────────── LOAD ROOMS ───────────────────────────
void loadRooms() {
    ifstream file("Room_Dataset.csv");
    if (!file.is_open()) {
        cout << "Error: Room_Dataset.csv not found!\n";
        rooms = 0;
        return;
    }

    string line;
    getline(file, line);
    rooms = 0;

    while (getline(file, line) && rooms < MAX) {
        stringstream ss(line);
        string cap;
        if (getline(ss, roomNames[rooms], ',')) {
            roomCapacity[rooms] = 50;
            if (getline(ss, cap, ',')) {
                try { roomCapacity[rooms] = stoi(cap); } catch (...) {}
            }
            rooms++;
        }
    }

    cout << "Loaded " << rooms << " rooms.\n";
    file.close();
}

// ─────────────────────────── LOAD TEACHERS ───────────────────────────
void loadTeachers() {
    ifstream file("Teachers_Room_Assigned_70.csv");
    if (!file.is_open()) {
        cout << "Error: Teachers_Room_Assigned_70.csv not found!\n";
        teacherCount = 0;
        return;
    }

    string line;
    getline(file, line);
    teacherCount = 0;

    while (getline(file, line) && teacherCount < MAX) {
        if (!line.empty() && line.back() == '\r') line.pop_back();

        stringstream ss(line);
        string name, rank, val;
        getline(ss, name, ',');
        getline(ss, rank, ',');

        teachers[teacherCount] = name;
        teacherRank[teacherCount] = rank;

        for (int s = 0; s < MAX_SLOTS; s++) {
            teacherAvailability[teacherCount][s] = 1;
            if (getline(ss, val, ',')) {
                try { teacherAvailability[teacherCount][s] = stoi(val); }
                catch (...) {}
            }
        }

        teacherCount++;
    }

    cout << "Loaded " << teacherCount << " teachers.\n";
    file.close();
}

// ─────────────────────────── SAVE TEACHERS CSV ───────────────────────────
void saveTeachersCSV() {
    ofstream file("Teachers_Room_Assigned_70.csv");
    file << "Teacher,Rank";
    for (int s = 0; s < 11; s++) file << ",Slot" << s;
    file << "\n";

    for (int t = 0; t < teacherCount; t++) {
        file << teachers[t] << "," << teacherRank[t];
        for (int s = 0; s < 11; s++) file << "," << teacherAvailability[t][s];
        file << "\n";
    }

    file.close();
    cout << "\n✅ SAVED to Teachers_Room_Assigned_70.csv\n";
}

// ─────────────────────────── INTERACTIVE EDITOR ───────────────────────────
void interactiveAvailabilityEditor() {
    cout << "\n=== INTERACTIVE TEACHER AVAILABILITY EDITOR ===\n";
    loadTeachers();

    char choice;
    do {
        cout << "\nOPTIONS:\n"
             << "1. View all teachers & availability\n"
             << "2. Edit specific teacher availability\n"
             << "3. Set all Assistant Professors available\n"
             << "4. Save and continue to timetable\n"
             << "5. Exit\n"
             << "Enter choice (1-5): ";

        cin >> choice;
        cin.ignore();

        if (choice == '1') {
            for (int t = 0; t < teacherCount; t++) {
                cout << "\n" << (t + 1) << ". " << teachers[t]
                     << " (" << teacherRank[t] << ")\n   Slots: ";
                for (int s = 0; s < 11; s++)
                    cout << teacherAvailability[t][s] << " ";
                cout << "\n";
            }
        }
        else if (choice == '2') {
            cout << "Enter teacher number (1-" << teacherCount << "): ";
            int t;
            cin >> t;
            t--;

            if (t < 0 || t >= teacherCount) {
                cout << "Invalid!\n";
                continue;
            }

            cout << "\n" << teachers[t] << " (" << teacherRank[t] << ")\nCurrent: ";
            for (int s = 0; s < 11; s++) cout << teacherAvailability[t][s] << " ";
            cout << "\nEnter slot number to change (0-10, or -1 to finish): ";

            int slotNum;
            while (cin >> slotNum, slotNum >= 0 && slotNum < 11) {
                int status;
                cout << "Slot " << slotNum << " (0=busy, 1=available): ";
                cin >> status;
                teacherAvailability[t][slotNum] = (status == 1) ? 1 : 0;
                cout << "Updated: ";
                for (int s = 0; s < 11; s++) cout << teacherAvailability[t][s] << " ";
                cout << "\nNext slot (-1 to finish): ";
            }
        }
        else if (choice == '3') {
            int count = 0;
            for (int t = 0; t < teacherCount; t++) {
                if (getRank(teacherRank[t]) == ASSISTANT_PROF) {
                    for (int s = 0; s < MAX_SLOTS; s++) teacherAvailability[t][s] = 1;
                    count++;
                }
            }
            cout << "Set " << count << " Assistant Professors fully available!\n";
        }
        else if (choice == '4') {
            saveTeachersCSV();
            break;
        }
        else if (choice == '5') {
            cout << "Exiting.\n";
            exit(0);
        }
    } while (true);
}

// ─────────────────────────── SLOT SOLVER ───────────────────────────
bool isSafe(int subject, int s) {
    for (int i = 0; i < n; i++) {
        if (conflict[subject][i] && slot[i] == s) return false;
    }
    return true;
}

bool solveSlots(int subject, int maxSlots) {
    if (subject == n) return true;

    for (int s = 0; s < maxSlots; s++) {
        if (isSafe(subject, s)) {
            slot[subject] = s;
            if (solveSlots(subject + 1, maxSlots)) return true;
            slot[subject] = -1;
        }
    }
    return false;
}

int generateSlots() {
    for (int i = 0; i < n; i++) slot[i] = -1;

    for (int s = 1; s <= n; s++) {
        if (solveSlots(0, s)) {
            cout << "Found solution with " << s << " slots.\n";
            return s;
        }
    }

    cout << "No solution found, using " << n << " slots.\n";
    return n;
}

// ─────────────────────────── BEST-FIT ROOM ALLOCATION ───────────────────────────
void chooseBestRoomsRec(int idx,
                        int currentSum,
                        int currentRoomsUsed,
                        int requiredStudents,
                        int currentUsed[],
                        int bestUsed[],
                        int &bestSum,
                        int &bestRoomCount) {
    if (currentSum >= requiredStudents) {
        if (currentSum < bestSum ||
            (currentSum == bestSum && currentRoomsUsed < bestRoomCount)) {
            bestSum = currentSum;
            bestRoomCount = currentRoomsUsed;
            for (int i = 0; i < rooms; i++) bestUsed[i] = currentUsed[i];
        }
        return;
    }

    if (idx == rooms) return;
    if (currentSum > bestSum) return;
    if (currentSum == bestSum && currentRoomsUsed >= bestRoomCount) return;

    currentUsed[idx] = 1;
    chooseBestRoomsRec(idx + 1,
                       currentSum + roomCapacity[idx],
                       currentRoomsUsed + 1,
                       requiredStudents,
                       currentUsed,
                       bestUsed,
                       bestSum,
                       bestRoomCount);

    currentUsed[idx] = 0;
    chooseBestRoomsRec(idx + 1,
                       currentSum,
                       currentRoomsUsed,
                       requiredStudents,
                       currentUsed,
                       bestUsed,
                       bestSum,
                       bestRoomCount);
}

void allocateRoomsBestFit(int used[], int &sum, int requiredStudents) {
    int currentUsed[MAX] = {0};
    int bestUsed[MAX] = {0};
    int totalCapacity = 0;

    for (int i = 0; i < rooms; i++) totalCapacity += roomCapacity[i];

    if (totalCapacity < requiredStudents) {
        for (int i = 0; i < rooms; i++) used[i] = 1;
        sum = totalCapacity;
        return;
    }

    int bestSum = INT_MAX;
    int bestRoomCount = INT_MAX;

    chooseBestRoomsRec(0, 0, 0, requiredStudents,
                       currentUsed, bestUsed,
                       bestSum, bestRoomCount);

    for (int i = 0; i < rooms; i++) used[i] = bestUsed[i];
    sum = bestSum;
}

// ─────────────────────────── TEACHER PRIORITY ───────────────────────────
// Rule:
// Assistant preferred normally
// If assistant load becomes exactly 1, associate gets next preference
// Professor after that
// Invalid/other ranks are ignored
int getTeacherPriority(int t, int currentSlot, bool availableOnly) {
    RankType rank = getRank(teacherRank[t]);

    if (rank == INVALID_RANK) return 999999;

    if (availableOnly && teacherAvailability[t][currentSlot] == 0)
        return 999999;

    int priority = 999999;

    switch (rank) {
        case ASSISTANT_PROF:
            if (teacherUsage[t] == 1)
                priority = 100 + teacherUsage[t];
            else
                priority = teacherUsage[t];
            break;

        case ASSOCIATE_PROF:
            priority = 10 + teacherUsage[t];
            break;

        case PROFESSOR:
            priority = 50 + teacherUsage[t];
            break;

        default:
            return 999999;
    }

    if (!availableOnly && teacherAvailability[t][currentSlot] == 0)
        priority += 1000;

    return priority;
}

// ─────────────────────────── DISPLAY TIMETABLE ───────────────────────────
void display(int totalSlots) {
    cout << "\n===== OPTIMAL TIMETABLE =====\n";
    cout << "(Best-Fit Room Allocation + Custom Teacher Priority)\n";

    memset(teacherUsage, 0, sizeof(teacherUsage));

    for (int s = 0; s < totalSlots; s++) {
        cout << "\n--- Slot " << (s + 1) << " ---\n";
        bool hasSubjects = false;

        for (int i = 0; i < n; i++) {
            if (slot[i] != s) continue;
            hasSubjects = true;

            int used[MAX] = {0};
            int sum = 0;
            allocateRoomsBestFit(used, sum, students[i]);

            cout << "\nSubject: " << subjects[i]
                 << "  (Students: " << students[i]
                 << ", Allocated Capacity: " << sum << ")\n";

            priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>>> pq;
            int availableTeachers = 0;

            for (int t = 0; t < teacherCount; t++) {
                if (getRank(teacherRank[t]) == INVALID_RANK) continue;

                if (teacherAvailability[t][s] == 1) {
                    int pri = getTeacherPriority(t, s, true);
                    if (pri != 999999) {
                        pq.push({pri, t});
                        availableTeachers++;
                    }
                }
            }

            bool forcedAssignment = false;
            if (availableTeachers == 0) {
                forcedAssignment = true;
                cout << "  [Fallback: teachers assigned even if unavailable]\n";

                for (int t = 0; t < teacherCount; t++) {
                    if (getRank(teacherRank[t]) == INVALID_RANK) continue;

                    int pri = getTeacherPriority(t, s, false);
                    if (pri != 999999) {
                        pq.push({pri, t});
                    }
                }
            }

            if (pq.empty()) {
                cout << "  No valid teachers available.\n";
                continue;
            }

            for (int r = 0; r < rooms; r++) {
                if (!used[r]) continue;
                if (pq.empty()) break;

                auto [pri, t] = pq.top();
                pq.pop();

                teacherUsage[t]++;

                cout << "  Room: " << roomNames[r]
                     << " (Capacity: " << roomCapacity[r] << ")"
                     << "  |  " << teachers[t]
                     << "  (" << teacherRank[t]
                     << ", Load: " << teacherUsage[t];

                if (forcedAssignment && teacherAvailability[t][s] == 0)
                    cout << ", Forced";
                cout << ")\n";

                int newPri = getTeacherPriority(t, s, !forcedAssignment);
                if (newPri != 999999) {
                    pq.push({newPri, t});
                }
            }
        }

        if (!hasSubjects) cout << "No subjects this slot.\n";
    }

    cout << "\n=== FINAL WORKLOAD SUMMARY ===\n";
    int asstTotal = 0, assocTotal = 0, profTotal = 0;

    for (int t = 0; t < teacherCount; t++) {
        if (teacherUsage[t] == 0) continue;
        if (getRank(teacherRank[t]) == INVALID_RANK) continue;

        cout << "  " << teachers[t]
             << " (" << teacherRank[t] << "): "
             << teacherUsage[t] << " room duties\n";

        switch (getRank(teacherRank[t])) {
            case ASSISTANT_PROF: asstTotal += teacherUsage[t]; break;
            case ASSOCIATE_PROF: assocTotal += teacherUsage[t]; break;
            
            default: break;
        }
    }

    int total = asstTotal + assocTotal ;
    if (total > 0) {
        cout << "\nAsst Prof:    " << asstTotal << " (" << (asstTotal * 100 / total) << "%)\n"
             << "Assoc Prof:   " << assocTotal << " (" << (assocTotal * 100 / total) << "%)\n"
             ;
    }
}

// ─────────────────────────── MAIN ───────────────────────────
int main() {
    cout << "=== EXAM DUTY ALLOCATOR WITH INTERACTIVE EDITOR ===\n\n";

    loadSubjects();
    loadConflict();
    loadRooms();

    if (n == 0) {
        cout << "No subjects loaded. Exiting.\n";
        return 1;
    }

    interactiveAvailabilityEditor();

    int totalSlots = generateSlots();
    cout << "\nMinimum Slots Required: " << totalSlots << "\n\n";

    display(totalSlots);

    cout << "\n✅ Timetable generated successfully!\n";
    return 0;
}