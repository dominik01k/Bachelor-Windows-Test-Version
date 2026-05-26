from python_strategies.player.autonomous.base_strategy_autonomous import BaseStrategyPlayerAutonomous

class ExamplePlayerPythonAIAutonomous(BaseStrategyPlayerAutonomous):
    def get_name(self):
        return "Player AI Autonomous"
    
    def decide_action(self, game_state_json: str, player_state_json: str) -> dict | None:
        return {
            "type": "MoveForward",
            "value": 20
        }
