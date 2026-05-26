class BaseStrategyPlayerWithTeamContext:
    def get_name(self) -> str:
        raise NotImplementedError

    def decide_action(self, game_state_json: str, player_state_json: str, target_position_json: str) -> dict | None:
        raise NotImplementedError
