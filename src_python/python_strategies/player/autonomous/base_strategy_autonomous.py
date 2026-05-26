class BaseStrategyPlayerAutonomous:
    def get_name(self) -> str:
        raise NotImplementedError

    
    def decide_action(self, game_state_json: str, player_state_json: str) -> dict | None:
        raise NotImplementedError

