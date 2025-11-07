/*
Copyright (C) 2025  Neo Reinmann (neoreinmann@gmail.com)
*/

#include "Raytracing.h"
#include "geometry/Vec3fa.h"
#include <string>
#include <iostream>
#include "lib/random.h"
#include "lib/XMLData.h"
#include <chrono>
#include <utility>
#include <cmath>
#include <vector>
#include <unordered_map>
#include <functional> // For std::hash
#include <fstream>
#include <future>
#include <chrono>
#include <sstream>     // <-- CSV parsing
#include <optional>    // <-- std::optional
#include <iomanip>     // <-- CSV formatting
#include <array>
#include "mirror_module/LobsterEyeOptics.h"


struct hit_entry{
    hit_entry(int i, Ray fa) : index(i), hit(std::move(fa)) { }
    int index;
    Ray hit;
};

std::string print_rt_hist(std::vector<shape_id> rt_hist){
    std::string print_out;
    for (const shape_id &shapeId : rt_hist) {
        print_out.append(
                std::to_string(shapeId.origin.x) + " " + std::to_string(shapeId.origin.y) + " " + std::to_string(shapeId.origin.z) + " " +
                std::to_string(shapeId.direction.x) + " " + std::to_string(shapeId.direction.y) + " " + std::to_string(shapeId.direction.z) + " " +
                std::to_string(shapeId.id) + " "
        );
    }
    return print_out;
}

double generateRandomDouble(double m, double n) {
    double uniform_number = easy_uniform_random();
    return m + (n-m) * uniform_number;
}

void writeUnorderedMapToTextFile(const std::vector<hit_entry>& hits, const std::string& filename) {
    std::cout << "Start writing into file.\n";
    std::ofstream ofs(filename);
    if (!ofs) {
        std::cerr << "Error opening file for writing!" << std::endl;
        return;
    }
    for (const auto& hit : hits) {
        ofs << hit.index << " "
            << hit.hit.position().x << " "
            << hit.hit.position().y << " "
            << print_rt_hist(hit.hit.raytracing_history) << "\n";
    }
    ofs.close();
}

void simulate_location(const std::unique_ptr<MirrorModule> &telescope, const int n_photons, const double dir_x, const double dir_y, double energy=1000, int idx=0) {
    using std::chrono::high_resolution_clock;
    auto t1 = high_resolution_clock::now();
    int lb = 200, ub = -200;
    std::vector<hit_entry> hits;
    for (int i = 0; i < n_photons; i++) {
        double x = generateRandomDouble(lb, ub);
        double y = generateRandomDouble(lb, ub);
        Vec3fa direction(dir_x, dir_y, -1);
        auto ray = Ray(Vec3fa(x, y, telescope->get_focal_length()*2+200), direction, energy);

        std::optional<Ray> hit = telescope->ray_trace(ray);
        if (!hit) continue;
        hits.emplace_back(i, *hit);
    }
    auto t2 = high_resolution_clock::now();
    std::chrono::duration<double, std::milli> ms_double = t2 - t1;
    std::cout << "time for " << n_photons << " photons: " << ms_double.count() << "ms\n";
    t1 = high_resolution_clock::now();
    const std::string filename = std::to_string(idx) + "_" + std::string("point_off_focus_x") + std::to_string(dir_x) + "_y" + std::to_string(dir_y) + ".txt";
    writeUnorderedMapToTextFile(hits, filename);
    t2 = high_resolution_clock::now();
    ms_double = t2 - t1;
    std::cout << "time for writing " << n_photons << " photons: " << ms_double.count() << "ms\n";
}



std::unique_ptr<MirrorModule> create_telescope(const std::string& path)
{
    XMLData xml_data{path};
    auto raytracing = xml_data.child("telescope").child("raytracer");
    std::string telescope_type = raytracing.child("type").attributeAsString("type");
    if (telescope_type == "wolter")
        return std::make_unique<Wolter>(xml_data);
    if (telescope_type == "lobster_eye")
        return std::make_unique<LobsterEyeOptics>(xml_data);
    throw std::runtime_error("Unknown mirror_module type: " + telescope_type);
}

void simulate_psfs_single_thread(const std::unique_ptr<MirrorModule> &telescope, int n_photons) {
    for (int l = 0; l < 1; l++) {
        for (int k = 0; k < 1; k++) {
            simulate_location(telescope, n_photons, 0.002*k, 0.0012*l);
        }
    }
}


/* ----------------------------- NEW: CSV retrace ----------------------------- */

struct CSVPhoton {
    int    id = -1;
    double ex = 0, ey = 0, ez = 0;
    double dx = 0, dy = 0, dz = -1;
};

static inline std::string trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

static bool parse_csv_photon_line(const std::string& line, CSVPhoton& out)
{
    if (line.empty()) return false;
    if (line[0] == '#') return false;

    // skip header (starts with "ray_id")
    if (line.rfind("ray_id", 0) == 0) return false;

    std::vector<std::string> tok;
    tok.reserve(20);
    std::stringstream ss(line);
    std::string item;
    while (std::getline(ss, item, ',')) tok.push_back(trim(item));

    // We only require first 7 fields:
    // 0: ray_id, 1: emit_x_mm, 2: emit_y_mm, 3: emit_z_mm,
    // 4: dir_x, 5: dir_y, 6: dir_z
    if (tok.size() < 7) return false;

    try {
        out.id = std::stoi(tok[0]);
        out.ex = std::stod(tok[1]);
        out.ey = std::stod(tok[2]);
        out.ez = std::stod(tok[3]);
        out.dx = std::stod(tok[4]);
        out.dy = std::stod(tok[5]);
        out.dz = std::stod(tok[6]);
    } catch (...) {
        return false;
    }
    return true;
}

void retrace_from_csv_same_photons(const std::unique_ptr<MirrorModule>& telescope,
                                   const std::string& inCsvPath,
                                   const std::string& outCsvPath)
{
    std::ifstream in(inCsvPath);
    if (!in.is_open()) {
        std::cerr << "[CSV Retrace] ERROR: cannot open input CSV: " << inCsvPath << "\n";
        return;
    }
    std::ofstream out(outCsvPath);
    if (!out.is_open()) {
        std::cerr << "[CSV Retrace] ERROR: cannot open output CSV: " << outCsvPath << "\n";
        return;
    }

    out << std::setprecision(9) << std::fixed;
    out
            << "ray_id,"
            << "emit_x_mm,emit_y_mm,emit_z_mm,"
            << "dir_x,dir_y,dir_z,"
            << "hit_sensor,"
            << "hit_x_mm,hit_y_mm,hit_z_mm,"
            << "history_len,"
            << "history_flat\n";

    uint64_t total = 0, hits = 0;
    std::string line;
    while (std::getline(in, line)) {
        CSVPhoton p;
        if (!parse_csv_photon_line(line, p)) continue;
        total++;

        // Create lvalues (no temporaries) for Ray ctor
        Vec3fa o((float)p.ex, (float)p.ey, (float)p.ez);
        Vec3fa d((float)p.dx, (float)p.dy, (float)p.dz);
        Ray ray(o, d, 277.0f);

        std::optional<Ray> hit = telescope->ray_trace(ray);

        if (hit) {
            hits++;
            const Ray& hr = *hit;
            out << p.id << ","
                << p.ex << "," << p.ey << "," << p.ez << ","
                << p.dx << "," << p.dy << "," << p.dz << ","
                << 1 << ","
                << hr.position().x << "," << hr.position().y << "," << hr.position().z << ","
                << hr.raytracing_history.size() << ",\""
                << print_rt_hist(hr.raytracing_history) << "\"\n";
        } else {
            out << p.id << ","
                << p.ex << "," << p.ey << "," << p.ez << ","
                << p.dx << "," << p.dy << "," << p.dz << ","
                << 0 << ",,,,"
                << 0 << ",\"\"\n";
        }
    }

    std::cout << "[CSV Retrace] traced " << total
              << " photons; sensor hits = " << hits
              << " -> wrote " << outCsvPath << "\n";
}

void simulate_psf_moving_around(const std::unique_ptr<MirrorModule> &telescope, int n_photons) {
    int idx=0;
    for (int i = 0; i <= 100; i++) {
        simulate_location(telescope, n_photons, 0.0001*i, 0,1000.0,idx);
        idx++;
    }
    for (int i = 0; i <= 100; i++) {
        simulate_location(telescope, n_photons, 0.0001*100,0.0001*i,1000.0, idx);
        idx++;
    }

    for (int i = 100; i >= 0; i--) {
        simulate_location(telescope, n_photons, 0.0001*i, 0.0001*i,1000.0, idx);
        idx++;
    }
}

void simulate_2D(const std::unique_ptr<MirrorModule> &telescope, int n_photons) {
    simulate_location(telescope, n_photons, 0, 0, 1000.0);
}
/* --------------------------- end NEW: CSV retrace --------------------------- */
#include <filesystem>
int main(int argc, char *argv[]) {
    std::cerr << "CWD  = " << std::filesystem::current_path() << "\n";
    std::cerr << "ARGV =";

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <telescope.xml> \n";
        return -1;
    }
    const std::string path = argv[1];

    using std::chrono::high_resolution_clock;
    auto t1 = high_resolution_clock::now();
    std::unique_ptr<MirrorModule> telescope = create_telescope(path);
    auto t2 = high_resolution_clock::now();
    std::chrono::duration<double, std::milli> ms_double = t2 - t1;
    std::cout << "Time loading and creating mirror_module: " << ms_double.count() << "ms\n";

    XMLData xml_data{path};
    auto raytracing = xml_data.child("telescope").child("raytracer");
    int n_photons =  raytracing.child("simulation_details").attributeAsInt("n_photons");
    simulate_psfs_single_thread(telescope, n_photons);
}
