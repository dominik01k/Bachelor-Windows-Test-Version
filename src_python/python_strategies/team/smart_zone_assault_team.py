import json
from math import dist
from collections import defaultdict
from python_strategies.team.base_strategy_team import BaseStrategyTeam  # falls du sie extern gespeichert hast

class SmartZoneAssaultTeam(BaseStrategyTeam):
    def get_name(self) -> str:
        return "SmartZoneAssault"

    def decide_action(self, game_state_str, team_id, player_map_str) -> dict:
        game_state = json.loads(game_state_str)
        player_positions = json.loads(player_map_str)
        zones = game_state["zones"]
        enemies = game_state["enemyPositions"]
        enemy_bullets = game_state["enemyBulletPositions"]

        def distance(a, b):
            return ((a["x"] - b["x"]) ** 2 + (a["y"] - b["y"]) ** 2) ** 0.5

        def danger_near(pos, threats, radius=5):
            return any(distance(pos, t) <= radius for t in threats)

        def score_zone(zone, player_pos):
            score = 0
            progress = zone["teamProgress"]
            our_progress = progress.get(str(team_id), 0.0)
            enemy_progress = sum(p for tid, p in progress.items() if int(tid) != team_id)

            if our_progress >= 100.0:
                score -= 100
            elif enemy_progress > our_progress:
                score += 30
            else:
                score += 10

            # Fortschritt
            score += (100 - our_progress) * 0.5

            # Entfernung
            dist_to_zone = distance(player_pos, zone["position"])
            score -= dist_to_zone * 0.4  # näher ist besser

            # Gefahr
            if danger_near(zone["position"], enemies, radius=6):
                score -= 20
            if danger_near(zone["position"], enemy_bullets, radius=4):
                score -= 30

            return score

        target_assignments = {}
        zone_assigned = defaultdict(int)

        for player_id, pos in player_positions.items():
            best_zone = None
            best_score = float('-inf')

            for zone in zones:
                s = score_zone(zone, pos)
                if s > best_score:
                    best_score = s
                    best_zone = zone

            if best_zone:
                zone_assigned[(best_zone["position"]["x"], best_zone["position"]["y"])] += 1
                target_assignments[int(player_id)] = {
                    "x": best_zone["position"]["x"],
                    "y": best_zone["position"]["y"]
                }

        return target_assignments
