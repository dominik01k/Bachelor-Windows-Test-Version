import math

DEBUG_PRINT = True

def normalize(vx, vy):
    length = math.sqrt(vx * vx + vy * vy)
    if length == 0:
        return 0.0, 0.0
    return vx / length, vy / length

def get_xy(v):
    if isinstance(v, dict):
        return float(v.get("x", 0.0)), float(v.get("y", 0.0))
    return float(v[0]), float(v[1])

def zone_features(sample, max_zones=4):
    zones = sample.get("zones", [])
    player_x, player_y = get_xy(sample["playerPos"])
    my_team = sample.get("teamID", 0)
    
    if DEBUG_PRINT:
        print(f"  [ZONES] Found: {len(zones)} | MyTeamID: {my_team}")

    zone_data = []
    for i, z in enumerate(zones):
        zx, zy = get_xy(z["position"])
        radius = float(z.get("radius", 0.0))
        dx, dy = zx - player_x, zy - player_y
        dist = math.sqrt(dx*dx + dy*dy)
        
        team_progress = z.get("teamProgress", {})
        self_prog = float(team_progress.get(str(my_team), 0.0))
        
        enemy_prog = 0.0
        for tid, prog in team_progress.items():
            if int(tid) != my_team:
                enemy_prog = max(enemy_prog, float(prog))
        
        is_contested = 1.0 if enemy_prog > 0 and self_prog > 0 else 0.0
        zone_data.append((dist, dx, dy, radius, self_prog, enemy_prog, self_prog - enemy_prog, is_contested))
        
        if DEBUG_PRINT and i < 3:
            print(f"    -> Zone {i} at ({zx:.0f}, {zy:.0f}): Dist={dist:.1f}, MyProg={self_prog:.2f}, EnemyMax={enemy_prog:.2f}")

    zone_data.sort(key=lambda z: z[0])
    features = []
    for i in range(max_zones):
        if i < len(zone_data):
            features.extend(zone_data[i])
        else:
            features.extend([0.0] * 8)
    return features

def extract_enemy_features(sample, max_enemies=3):
    player_x, player_y = get_xy(sample["playerPos"])
    rotation = float(sample.get("playerRotation", 0.0))
    enemies = sample.get("visibleEnemies", [])
    
    if DEBUG_PRINT:
        print(f"  [ENEMIES] Found: {len(enemies)}")

    enemy_data = []
    for i, e in enumerate(enemies):
        ex, ey = get_xy(e)
        dx, dy = ex - player_x, ey - player_y
        dist = math.sqrt(dx * dx + dy * dy)

        angle_rad = math.atan2(dy, dx) - math.radians(rotation)
        while angle_rad > math.pi: angle_rad -= 2 * math.pi
        while angle_rad < -math.pi: angle_rad += 2 * math.pi
        
        enemy_data.append((dist, angle_rad))
        
        if DEBUG_PRINT and i < 3:
            print(f"    -> Enemy {i} at ({ex:.0f}, {ey:.0f}): Dist={dist:.1f}, RelAngle={math.degrees(angle_rad):.1f}°")
    
    enemy_data.sort(key=lambda x: x[0])
    features = []
    for i in range(max_enemies):
        if i < len(enemy_data):
            dist, angle = enemy_data[i]
            features.extend([dist, math.sin(angle), math.cos(angle)])
        else:
            features.extend([0.0, 0.0, 0.0])
    return features

def bullet_features(sample, max_bullets=5):
    px, py = get_xy(sample["playerPos"])
    bullets = sample.get("visibleBullets", [])
    enemies = sample.get("visibleEnemies", [])
    
    if DEBUG_PRINT:
        print(f"  [BULLETS] Found: {len(bullets)}")

    bullet_data = []
    for i, b in enumerate(bullets):
        bx, by = get_xy(b)
        dx, dy = bx - px, by - py
        dist = math.sqrt(dx * dx + dy * dy)
        
        dir_to_player_x, dir_to_player_y = normalize(dx, dy)

        if enemies:
            closest_enemy = min(enemies, key=lambda e: (bx - get_xy(e)[0])**2 + (by - get_xy(e)[1])**2)
            ex, ey = get_xy(closest_enemy)
            dir_bullet_x, dir_bullet_y = normalize(bx - ex, by - ey)
        else:
            dir_bullet_x, dir_bullet_y = 0.0, 0.0
            
        impact_cos = dir_bullet_x * dir_to_player_x + dir_bullet_y * dir_to_player_y
        bullet_data.append((dist, dir_to_player_x, dir_to_player_y, dir_bullet_x, dir_bullet_y, impact_cos))
        
        if DEBUG_PRINT and i < 3:
            danger_level = "HIGH" if impact_cos < -0.8 else "LOW" # -1 bedeutet direkt auf Spieler zu
            print(f"    -> Bullet {i} at ({bx:.0f}, {by:.0f}): Dist={dist:.1f}, Danger={danger_level} (Cos={impact_cos:.2f})")
    
    bullet_data.sort(key=lambda b: b[0])
    features = []
    for i in range(max_bullets):
        if i < len(bullet_data):
            features.extend(bullet_data[i])
        else:
            features.extend([0.0] * 6)
    return features

def build_features(sample):
    state_map = {
        "StillWorking": 0.0,
        "Successful": 1.0,
        "Failed": -1.0,
        "Unknown": 0.0
    }
    
    px, py = get_xy(sample["playerPos"])
    tx, ty = get_xy(sample["targetPos"])
    rotation = float(sample.get("playerRotation", 0.0))
    can_shoot = 1.0 if sample.get("canShoot", False) else 0.0
    is_dead = 1.0 if sample.get("isDead", False) else 0.0
    raw_state = sample.get("lastCommandState", "Unknown")
    state_value = state_map.get(raw_state, 0.0)

    if DEBUG_PRINT:
        print("\n" + "="*50)
        print("--- [FEATURE BUILDER DIAGNOSIS] ---")
        print(f"  PLAYER: Pos({px:.1f}, {py:.1f}) | Rot: {rotation:.1f}°")
        print(f"  TARGET: Pos({tx:.1f}, {ty:.1f}) | State: {raw_state}")
        print(f"  STATUS: CanShoot={can_shoot} | IsDead={is_dead}")

    features = [px, py, rotation, tx, ty, can_shoot, is_dead, state_value]

    features.extend(extract_enemy_features(sample))
    features.extend(bullet_features(sample))
    features.extend(zone_features(sample))

    if DEBUG_PRINT:
        print(f"  RESULT: Vector length {len(features)}")
        print("="*50 + "\n")

    return features