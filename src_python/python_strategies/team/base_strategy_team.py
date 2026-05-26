import json
from typing import Any

class BaseStrategyTeam:
    def get_name(self) -> str:
        raise NotImplementedError("Strategy must implement get_name")

    def decide_action(self, game_state_str, team_id, player_map_str) -> dict:
        """
        Receives the state as a JSON string, parse it to a dict inside the function.
        Return any Python object that can be handled by C++ (e.g., dict, list, etc.).
        """
        raise NotImplementedError("Strategy must implement decide_action")
