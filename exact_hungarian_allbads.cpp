#include <algorithm>
#include <cassert>
#include <cmath>
#include <deque>
#include <exception>
#include <iomanip>
#include <iostream>
#include <limits>
#include <set>
#include <utility>
#include <vector>

struct point {
   double x, y;
};

double distance_magnitude(const point& p1, const point& p2) {
   double dx = p1.x - p2.x, dy = p1.y - p2.y;
   return dx * dx + dy * dy;
}

double distance(const point& p1, const point& p2) {
   return std::sqrt(distance_magnitude(p1, p2));
}

double reduced_cost(int p1, int p2, const std::vector<point>& points, const std::vector<double>& nearest) {
   return nearest[p1] + nearest[p2] - distance(points[p1], points[p2]);
}

std::set<int> find_bads(int a, int b, const std::vector<int>& matching, const std::vector<double>& beta) {
   std::set<int> res;
   for (int j = a; j < a + b; ++j) {
      if (matching[j] == -1 && beta[j] > 1e-15) {
         res.insert(j);
      }
   }
   return res;
}

void update_matching(std::vector<int>& matching, std::vector<int>& tree, int start, int a, int b) {
   std::vector<std::pair<int, int>> path;
   for (int p = start; tree[p] != -1; p = tree[p]) {
      path.emplace_back(p, tree[p]);
   }
   for (int c = 1 - path.size( ) % 2; c < path.size( ); c += 2) {
      for (int p : { path[c].first, path[c].second }) {
         if (matching[p] != -1) {
            matching[matching[p]] = -1;
         }
      }
      matching[path[c].first] = path[c].second;
      matching[path[c].second] = path[c].first;
   }
}

int main( ) try {
   int a, b;
   std::cin >> a >> b;

   std::vector<point> points(a + b);
   for (int v = 0; v < a + b; ++v) {
      std::cin >> points[v].x >> points[v].y;
   }

   std::vector<int> closest_v(a + b);
   std::vector<double> nearest(a + b, std::numeric_limits<double>::max( ));
   for (int i = 0; i < a; ++i) {
      for (int j = a; j < a + b; ++j) {
         double d = distance(points[i], points[j]);
         if (d < nearest[i]) {
            nearest[i] = d;
            closest_v[i] = j;
         }
         if (d < nearest[j]) {
            nearest[j] = d;
            closest_v[j] = i;
         }
      }
   }

   std::vector<int> matching(a + b, -1);
   std::vector<double> alpha(a, 0);
   std::vector<double> beta(a + b, std::numeric_limits<double>::lowest( ));
   for (int j = a; j < a + b; ++j) {
      for (int i = 0; i < a; ++i) {
         beta[j] = std::max(beta[j], reduced_cost(i, j, points, nearest));
      }
   }

   for (std::set<int> s; !(s = find_bads(a, b, matching, beta)).empty( );) {
      int ej = *std::min_element(s.begin( ), s.end( ), [&](int j1, int j2) {
         return beta[j1] < beta[j2];
      }); double epsilon = beta[ej];
      std::set<int> t, f;
      for (int i = 0; i < a; ++i) {
         f.insert(i);
      }
      std::vector<int> tree(a + b, -1);

      for (;;) {
         double delta = std::numeric_limits<double>::max( ); int di, dj;
         for (int i : f) {
            for (int j : s) {
               if (double check = alpha[i] + beta[j] - reduced_cost(i, j, points, nearest); check < delta) {
                  delta = check, di = i, dj = j;
               }
            }
         }

         // case 1
         if (std::abs(delta) <= 1e-15 && matching[di] == -1) {
            tree[di] = dj;
            update_matching(matching, tree, di, a, b);
            break;
         }

         // case 2
         if (std::abs(delta) <= 1e-15 && matching[di] != -1) {
            int kj = matching[di];
            tree[kj] = di, tree[di] = dj;
            f.erase(di), t.insert(di), s.insert(kj);
            if (beta[kj] < epsilon) {
               epsilon = beta[kj], ej = kj;
            }
            continue;
         }

         // case 3
         if (epsilon > delta) {
            for (int i : t) {
               alpha[i] += delta;
            }
            for (int j : s) {
               beta[j] -= delta;
            }
            epsilon -= delta;
            continue;
         }

         // case 4
         if (delta >= epsilon) {
            for (int i : t) {
               alpha[i] += epsilon;
            }
            for (int j : s) {
               beta[j] -= epsilon;
            }
            epsilon -= epsilon;
            update_matching(matching, tree, ej, a, b);
            break;
         }
      }
   }

   std::vector<std::pair<int, int>> used;
   std::vector<bool> covered(a + b, false);
   double total = 0;
   for (int i = 0; i < a + b; ++i) {
      if (!covered[i]) {
         int matched = (i < a && matching[i] != -1 ? matching[i] : closest_v[i]);
         used.emplace_back(i, matched);
         covered[i] = covered[matched] = true;
         total += distance(points[i], points[matched]);
      }
   }

   std::cout << used.size( ) << "\n";
   for (auto [p1, p2] : used) {
      std::cout << p1 << " " << p2 << "\n";
   }
   std::cout << std::setprecision(9) << std::fixed << total << "\n";
} catch (...) {
   return -1;
}
