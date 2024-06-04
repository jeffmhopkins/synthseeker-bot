#include <dpp/dpp.h>

long discord_channel = 0;
long discord_user_role = 0;
long discord_admin_role = 0;
long discord_admin_id = 0;
std::string discord_notification_role = "";
std::string discord_token = "";
std::string ninjam_server_password = "";

//Internal Temporary Server Variables
bool server_needs_announcement = false;
bool allow_announcements = true;
bool just_booted = true;
std::string ninjam_user_list = "";
std::string ninjam_user_list_get = "";
int get_status = 0;

int main() {
	dpp::cluster bot(discord_token);
	bot.on_log(dpp::utility::cout_logger());
	bot.on_slashcommand([&bot](const dpp::slashcommand_t & event) {
		bool command_allowed = false;
		bool admin_allowed = false;
		std::vector<dpp::snowflake> roles_list = event.command.member.get_roles();
		dpp::snowflake user_id = event.command.member.user_id;
		if(user_id == discord_admin_id) {
			command_allowed = true;
			admin_allowed = true;
		} else {
			for (int i=0; i<roles_list.size(); ++i) {
				if(roles_list[i] == discord_user_role) {
					command_allowed = true;
				}
				if(roles_list[i] == discord_user_role) {
					admin_allowed = true;
					command_allowed = true;
				}
			}
		}
		if(!command_allowed) {
			event.reply(dpp::message("You do not have proper roler membership for this command.").set_flags(dpp::m_ephemeral));
		} else {
			if (event.command.get_command_name() == "info") {
				event.reply(dpp::message("The Synthseeker NINJAM Server address is ninjam.synthseeker.online, port 2049, password is '" + ninjam_server_password + "', see http://ninjam.synthseeker.online/connect.html").set_flags(dpp::m_ephemeral));
			}
			if(!admin_allowed) {
				event.reply(dpp::message("You do not have proper role membership for this command.").set_flags(dpp::m_ephemeral));
			} else {
				if (event.command.get_command_name() == "settings") {
					std::string announcement_enable = std::get<std::string>(event.get_parameter("announcements"));
					if(announcement_enable == "announce_enable") {
						event.reply(dpp::message("Channel announcements are now enabled.").set_flags(dpp::m_ephemeral));
						allow_announcements = true;
					}
					if(announcement_enable == "announce_disable") {
						event.reply(dpp::message("Channel announcements are now disabled.").set_flags(dpp::m_ephemeral));
						allow_announcements = false;
					}
				}
				if (event.command.get_command_name() == "restart_server") {
					event.reply(dpp::message("Attempting to perform server restart...").set_flags(dpp::m_ephemeral));
					bot.request(
						"http://ninjam.synthseeker.online/util/discord.php?a=restart", 
						dpp::m_get, 
						[&bot](const dpp::http_request_completion_t & cc) {
							if(cc.status ==200) {
								bot.set_presence(dpp::presence(dpp::presence_status::ps_online, dpp::activity_type::at_custom, "Restarting Server..."));
							} else {
								bot.set_presence(dpp::presence(dpp::presence_status::ps_online, dpp::activity_type::at_custom, "Failed restart..."));
							}
						}, 
						"",
						"application/json",
						{{"Authorization", "Bearer tokengoeshere"}}
						);
				}
			}
		}
	});
	bot.on_ready([&bot](const dpp::ready_t& event) {
		//if (dpp::run_once<struct clear_bot_commands>()) {
		//	bot.global_bulk_command_delete();
		//}
		if (dpp::run_once<struct register_bot_commands>()) {
			bot.global_command_create(dpp::slashcommand("info", "Information about the NINJAM Server", bot.me.id));
			dpp::slashcommand settingscommand("settings", "change settings of the NINJAM bot.", bot.me.id);
			settingscommand.add_option(dpp::command_option(dpp::co_string, "announcements", "Allow server change messages in channel", true)
				.add_choice(dpp::command_option_choice("enable", std::string("announce_enable")))
				.add_choice(dpp::command_option_choice("disable", std::string("announce_disable")))
			);
			bot.global_command_create(settingscommand);
			bot.global_command_create(dpp::slashcommand("restart_server", "admin command to restart the server", bot.me.id));
			bot.set_presence(dpp::presence(dpp::presence_status::ps_online, dpp::activity_type::at_custom, "Server reboot success"));
		}
		bot.start_timer([&bot](const dpp::timer& timer){
			bot.request(
				"http://ninjam.synthseeker.online/util/discord.php?a=users", 
				dpp::m_get,
				[&bot](const dpp::http_request_completion_t & cc) {
					ninjam_user_list_get = cc.body;
					get_status = cc.status;
					if((ninjam_user_list_get.length() > 3) && (get_status ==200)) {
						if(ninjam_user_list.compare(ninjam_user_list_get) != 0) {
							ninjam_user_list = ninjam_user_list_get;
							server_needs_announcement = true;
						}
						get_status = 0;
					}
					if(server_needs_announcement) {
						if(ninjam_user_list.compare("No users") == 0) {
							if(allow_announcements) {
								if(!just_booted) {
									bot.message_create(dpp::message(discord_channel , discord_notification_role + ", Server empty. http://ninjam.synthseeker.online/recordings.php" ));
								}
							}
							bot.set_presence(dpp::presence(dpp::presence_status::ps_online, dpp::activity_type::at_custom, "Server empty"));
						} else {
							if(allow_announcements) {
								bot.message_create(dpp::message(discord_channel , discord_notification_role + ", " + ninjam_user_list + " online! http://ninjam.synthseeker.online/live/" ));
							}
							bot.set_presence(dpp::presence(dpp::presence_status::ps_online, dpp::activity_type::at_custom, ninjam_user_list + " online!"));
						}
						server_needs_announcement = false;
						just_booted = false;
					}
				},
				"",
				"application/json",
				{{"Authorization", "Bearer tokengoeshere"}}
			);
		}, 
		15);
	});
	bot.start(dpp::st_wait);
}
