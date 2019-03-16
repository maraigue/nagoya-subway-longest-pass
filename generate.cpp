#include <iostream>
#include <vector>
#include <deque>
#include <string>
#include <sstream>
#include <unordered_set>
#include <cassert>
#include <cstdlib>

struct Line{
	std::vector<size_t> stations;
	std::vector<int> distances;
};

struct Section{
	int distance;
	size_t station1, station2;
	Section(int d, size_t s1, size_t s2)
	: distance(d), station1(s1), station2(s2) {}
};

int main(int argc, char ** argv){
	int pattern, constraint;
	if(argc != 3 || (pattern = std::atoi(argv[1])) < 1 || pattern > 3 || ((constraint = std::atoi(argv[2])) < 1) || constraint > 2){
		std::cerr << "Usage: " << argv[0] << " ROUTE_PATTERN CONSTRAINT" << std::endl;
		std::cerr << "* ROUTE_PATTERN: 1 to 3" << std::endl;
		std::cerr << "* CONSTRAINT: 1 or 2" << std::endl;
		return -1;
	}

	const std::vector<std::string> AllStations = {
		"TAKABATA", "NAGOYA", "FUSHIMI", "SAKAE", "IMAIKE", "MOTOYAMA", "FUJIGAOKA",
		"KANAYAMA", "KAMIMAEZU", "HISAYAODORI", "HEIANDORI", "YAGOTO", "ARATAMABASHI", "NAGOYAKO",
		"KAMIOTAI", "MARUNOUCHI", "GOKISO", "AKAIKE", "NAKAMURAKUYAKUSHO", "TOKUSHIGE", "KAMIIIDA"
	};
	const size_t
		S_TAKABATA = 0,
		S_NAGOYA = 1,
		S_FUSHIMI = 2,
		S_SAKAE = 3,
		S_IMAIKE = 4,
		S_MOTOYAMA = 5,
		S_FUJIGAOKA = 6,
		S_KANAYAMA = 7,
		S_KAMIMAEZU = 8,
		S_HISAYAODORI = 9,
		S_HEIANDORI = 10,
		S_YAGOTO = 11,
		S_ARATAMABASHI = 12,
		S_NAGOYAKO = 13,
		S_KAMIOTAI = 14,
		S_MARUNOUCHI = 15,
		S_GOKISO = 16,
		S_AKAIKE = 17,
		S_NAKAMURAKUYAKUSHO = 18,
		S_TOKUSHIGE = 19,
		S_KAMIIIDA = 20;

	std::vector<Line> lines = {
		{{S_TAKABATA, S_NAGOYA, S_FUSHIMI, S_SAKAE, S_IMAIKE, S_MOTOYAMA, S_FUJIGAOKA},
		{0, 66, 80, 90, 117, 142, 206}},
		{{S_KANAYAMA, S_KAMIMAEZU, S_SAKAE, S_HISAYAODORI, S_HEIANDORI, S_MOTOYAMA, S_YAGOTO, S_ARATAMABASHI, S_KANAYAMA},
		{0, 16, 30, 34, 82, 141, 172, 207, 264}},
		{{S_KANAYAMA, S_NAGOYAKO},
		{0, 60}},
		{{S_KAMIOTAI, S_MARUNOUCHI, S_FUSHIMI, S_KAMIMAEZU, S_GOKISO, S_YAGOTO, S_AKAIKE},
		{0, 63, 70, 88, 119, 150, 204}},
		{{S_NAKAMURAKUYAKUSHO, S_NAGOYA, S_MARUNOUCHI, S_HISAYAODORI, S_IMAIKE, S_GOKISO, S_ARATAMABASHI, S_TOKUSHIGE},
		{0, 9, 24, 33, 63, 84, 118, 191}},
		{{S_HEIANDORI, S_KAMIIIDA},
		{0, 8}}
	};

	std::vector<Section> sections; // 区間の一覧

	std::vector< std::vector< std::vector<size_t> > > sectionIDs_by_station(AllStations.size()); // x[駅番号][路線番号] = {その駅を通る区間の一覧}
	std::vector<size_t> numSections_by_station(AllStations.size()); // x[駅番号] = 区間の数
	for(auto itss = sectionIDs_by_station.begin(); itss != sectionIDs_by_station.end(); ++itss){
		itss->resize(lines.size());
	}

	// 駅ごとに、接続している辺をまとめる
	std::cout << "set SECTIONS := {";
	bool init_section = true;
	for(size_t l = 0; l < lines.size(); ++l){
		for(size_t s = 0; s < lines[l].stations.size() - 1; ++s){
			size_t station1 = lines[l].stations[s];
			size_t station2 = lines[l].stations[s+1];
			sections.emplace_back(lines[l].distances[s+1] - lines[l].distances[s], station1, station2);
			size_t id = sections.size() - 1;
			sectionIDs_by_station[station1][l].push_back(id);
			sectionIDs_by_station[station2][l].push_back(id);
			++numSections_by_station[station1];
			++numSections_by_station[station2];

			if(init_section){
				init_section = false;
			}else{
				std::cout << ", ";
			}
			std::cout << "\"" << AllStations[station1] << "_" << AllStations[station2] << "\"";
		}
	}
	std::cout << "};" << std::endl;

	bool init_station = true;
	std::cout << "set STATIONS := {";
	for(auto itas = AllStations.begin(); itas != AllStations.end(); ++itas){
		if(init_station){
			init_station = false;
		}else{
			std::cout << ", ";
		}
		std::cout << '"' << *itas << '"';
	}
	std::cout << "};" << std::endl;

	// 制約条件を出力する準備
	std::vector< std::pair<size_t, size_t> > section_pairs;
	std::vector< std::vector<size_t> > section_pair_IDs_by_station(AllStations.size());
	std::vector<size_t> changing_stations;
	size_t max_degree = 0;
	std::vector<size_t> degrees(AllStations.size());

	for(size_t a = 0; a < AllStations.size(); ++a){
		// 駅の次数
		degrees[a] = 0;
		for(size_t l = 0; l < lines.size(); ++l){
			degrees[a] += sectionIDs_by_station[a][l].size();
		}

		// 乗換となる辺の組
		size_t has_change_constraint = 0;
		for(size_t l = 0; l < lines.size()-1; ++l){
			if(sectionIDs_by_station[a][l].empty()) continue;
			for(size_t m = l+1; m < lines.size(); ++m){
				if(sectionIDs_by_station[a][m].empty()) continue;
				has_change_constraint = 1;

				for(auto il = sectionIDs_by_station[a][l].begin(); il != sectionIDs_by_station[a][l].end(); ++il){
					for(auto im = sectionIDs_by_station[a][m].begin(); im != sectionIDs_by_station[a][m].end(); ++im){
						section_pairs.emplace_back(*il, *im);
						//std::cout << "	CHANGE " << AllStations[a == sections[*il].station1 ? sections[*il].station2 : sections[*il].station1] << "-" << AllStations[a == sections[*il].station1 ? a : sections[*il].station2] << "-" << AllStations[a == sections[*im].station1 ? sections[*im].station2 : sections[*im].station1] << std::endl;
						section_pair_IDs_by_station[a].push_back(section_pairs.size() - 1);
					}
				}
			}
		}

		
		if(degrees[a] > max_degree) max_degree = degrees[a];

		if(degrees[a] > 2 || has_change_constraint > 0){
			changing_stations.push_back(a);
		}
	}

	// 変数
	// (A) 区間を使っているか
	// (B) 駅を使っているか
	// (C) [区間2つの組で、異なる路線であって同一駅に接続しているもの]を使っているか
	auto var_section = [](size_t i){ return i + 1; };
	auto var_station = [&sections](size_t i){ return sections.size() + i + 1; };
	auto var_section_pair = [&sections, &AllStations](size_t i){ return sections.size() + AllStations.size() + i + 1; };

	std::cout << "var SEC{SECTIONS} binary;" << std::endl;
	std::cout << "var EME{STATIONS,1.." << max_degree << "} binary;" << std::endl;
	if(constraint >= 2){
		std::cout << "var CHANGE{1.." << section_pairs.size() << "} binary;" << std::endl;
	}

	// 最大化したい式
	std::cout << "maximize OBJ: ";
	for(size_t i = 0; i < sections.size(); ++i){
		if(i > 0) std::cout << " + ";
		std::cout << sections[i].distance << " * SEC[\"" << AllStations[sections[i].station1] << "_" << AllStations[sections[i].station2] << "\"]";
	}
	std::cout << ";" << std::endl;

	// 制約条件1（一筆書き制約）のダミー変数
	for(size_t t = 0; t < AllStations.size(); ++t){
		for(size_t m = 1; m <= max_degree; ++m){
			if(m <= degrees[t]){
				std::cout << "s.t. degree_ge" << m << "_" << AllStations[t] << ": -" << m << " + 1 - " << degrees[t] << " * EME[\"" << AllStations[t] << "\"," << m << "]";
				for(size_t l = 0; l < lines.size(); ++l){
					for(auto itss = sectionIDs_by_station[t][l].begin(); itss != sectionIDs_by_station[t][l].end(); ++itss){
						Section & s = sections[*itss];
						std::cout << " + SEC[\"" << AllStations[s.station1] << "_" << AllStations[s.station2] << "\"]";
					}
				}
				std::cout << ", <= 0;" << std::endl;

				std::cout << "s.t. degree_lt" << m << "_" << AllStations[t] << ": " << m << " - " << degrees[t] << " * (1 - EME[\"" << AllStations[t] << "\"," << m << "])";
				for(size_t l = 0; l < lines.size(); ++l){
					for(auto itss = sectionIDs_by_station[t][l].begin(); itss != sectionIDs_by_station[t][l].end(); ++itss){
						Section & s = sections[*itss];
						std::cout << " - SEC[\"" << AllStations[s.station1] << "_" << AllStations[s.station2] << "\"]";
					}
				}
				std::cout << ", <= 0;" << std::endl;
			}else{
				std::cout << "s.t. degree_no" << m << "_" << AllStations[t] << ": EME[\"" << AllStations[t] << "\"," << m << "], = 0;" << std::endl;
			}
		}
	}

	// 制約条件1（一筆書き制約）
	switch(pattern){
	case 1:
		for(size_t t = 0; t < AllStations.size(); ++t){
			if(degrees[t] >= 3){
				std::cout << "s.t. no_degree_ge3_" << AllStations[t] << ": EME[\"" << AllStations[t] << "\",3], = 0;" << std::endl;
			}
		}
		std::cout << "s.t. degree1_atmost2:";
		for(size_t t = 0; t < AllStations.size(); ++t){
			std::cout << " + EME[\"" << AllStations[t] << "\",1] - EME[\"" << AllStations[t] << "\",2]";
		}
		std::cout << ", <= 2;" << std::endl;
		break;
	case 2:
		for(size_t t = 0; t < AllStations.size(); ++t){
			if(degrees[t] >= 3){
				std::cout << "s.t. no_degree_ge4_" << AllStations[t] << ": EME[\"" << AllStations[t] << "\",4], = 0;" << std::endl;
			}
		}
		std::cout << "s.t. degree3_atmost2:";
		for(size_t t = 0; t < AllStations.size(); ++t){
			std::cout << " + EME[\"" << AllStations[t] << "\",3]";
		}
		std::cout << ", <= 1;" << std::endl;
		std::cout << "s.t. degree1_atmost0:";
		for(size_t t = 0; t < AllStations.size(); ++t){
			std::cout << " + EME[\"" << AllStations[t] << "\",1] - EME[\"" << AllStations[t] << "\",2]";
		}
		std::cout << ", <= 1;" << std::endl;
		break;
	case 3:
		for(size_t t = 0; t < AllStations.size(); ++t){
			if(degrees[t] >= 3){
				std::cout << "s.t. no_degree_ge4_" << AllStations[t] << ": EME[\"" << AllStations[t] << "\",4], = 0;" << std::endl;
			}
		}
		std::cout << "s.t. degree3_atmost2:";
		for(size_t t = 0; t < AllStations.size(); ++t){
			std::cout << " + EME[\"" << AllStations[t] << "\",3]";
		}
		std::cout << ", <= 2;" << std::endl;
		std::cout << "s.t. degree1_atmost0:";
		for(size_t t = 0; t < AllStations.size(); ++t){
			std::cout << " + EME[\"" << AllStations[t] << "\",1] - EME[\"" << AllStations[t] << "\",2]";
		}
		std::cout << ", <= 0;" << std::endl;
		break;
	default:
		std::cerr << "Not implemented yet: Route type " << pattern << std::endl;
		return -1;
	}

	// 制約条件2（乗換回数制約）のダミー変数
	if(constraint >= 2){
		for(size_t st = 0; st < section_pairs.size(); ++st){
			size_t id_s1 = section_pairs[st].first;
			size_t id_s2 = section_pairs[st].second;
			Section s1 = sections[id_s1];
			Section s2 = sections[id_s2];
			size_t a = -1;
			if(s1.station1 == s2.station1 || s1.station1 == s2.station2){
				a = s1.station1;
			}else if(s1.station2 == s2.station1 || s1.station2 == s2.station2){
				a = s1.station2;
			}else{
				assert(false);
			}

			std::cout << "s.t. changingStation" << (st+1) << ": -1 <= 2 * CHANGE[" << (st+1) << "] - SEC[\"" << AllStations[sections[id_s1].station1] << "_" << AllStations[sections[id_s1].station2] << "\"] - SEC[\"" << AllStations[sections[id_s2].station1] << "_" << AllStations[sections[id_s2].station2] << "\"] <= 0;" << std::endl;
		}

		// 制約条件2（乗換回数制約）
		std::cout << "s.t. allChangingStations: 0";
		for(size_t i = 0; i < section_pairs.size(); ++i){
			std::cout << " + CHANGE[" << (i+1) << "]";
		}
		std::cout << ", <= " << (pattern*2 + 1) << ";" << std::endl;
	}

	// 最適解を得る & 出力
	std::cout << "solve;" << std::endl;

	std::cout << "for{sec in SECTIONS}{" << std::endl;
	std::cout << "    printf (if SEC[sec] > 0.5 then (sec & \"\\n\") else \"\");" << std::endl;
	std::cout << "}" << std::endl;

	std::cout << "printf \"Total length: %f\\n\", OBJ;" << std::endl;
	std::cout << "end;" << std::endl;

	return 0;
}
