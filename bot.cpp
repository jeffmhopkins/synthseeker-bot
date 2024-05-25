#include <dpp/dpp.h>

//version v1.2

long discord_channel = 0123456789012345678; //channelID for announcements NEEDS UPDATED
std::string discord_notification_role = "<@&0123456789012345678>"; //Role to tag NEEDS UPDATED
std::string discord_token = "TOKEN"; //Bot Token NEEDS UPDATED

//Internal Temporary Server Variables
bool server_needs_announcement = false;
bool allow_announcements = true;
std::string ninjam_user_list = "";
std::string ninjam_user_list_get = "";
int get_status = 0;

int main() {
    dpp::cluster bot(discord_token);

    bot.on_log(dpp::utility::cout_logger());

    bot.on_slashcommand([&bot](const dpp::slashcommand_t & event) {
        if (event.command.get_command_name() == "info") {
			event.reply(dpp::message("The Synthseeker NINJAM Server address is ninjam.synthseeker.online:2049").set_flags(dpp::m_ephemeral));
        }
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
    });
 
    bot.on_ready([&bot](const dpp::ready_t& event) {
	//if (dpp::run_once<struct clear_bot_commands>()) {
        //    bot.global_bulk_command_delete();
        //}
		
        if (dpp::run_once<struct register_bot_commands>()) {
            bot.global_command_create(dpp::slashcommand("info", "Information about the NINJAM Server", bot.me.id));
			dpp::slashcommand settingscommand("settings", "change settings of the NINJAM bot.", bot.me.id);
            settingscommand.add_option(
                dpp::command_option(dpp::co_string, "announcements", "Allow server change messages in channel", true)
                    .add_choice(dpp::command_option_choice("enable", std::string("announce_enable")))
                    .add_choice(dpp::command_option_choice("disable", std::string("announce_disable")))
            );
 
            bot.global_command_create(settingscommand);
        }

        bot.start_timer([&bot](const dpp::timer& timer){
			bot.request(
				"http://ninjam.synthseeker.online/util/discord.php?a=users", dpp::m_get, [&bot](const dpp::http_request_completion_t & cc) {
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
								bot.message_create(dpp::message(discord_channel , discord_notification_role + ", Server empty. http://ninjam.synthseeker.online/recordings.php" ));
							}
							bot.set_presence(dpp::presence(dpp::presence_status::ps_online, dpp::activity_type::at_custom, "Server empty."));
						} else {
							if(allow_announcements) {
								bot.message_create(dpp::message(discord_channel , discord_notification_role + ", " + ninjam_user_list + " online! http://ninjam.synthseeker.online/live/" ));
							}
							bot.set_presence(dpp::presence(dpp::presence_status::ps_online, dpp::activity_type::at_custom, ninjam_user_list + " online!"));
						}
						server_needs_announcement = false;
					}
				},
				"",
				"application/json",
				{
					{"Authorization", "Bearer tokengoeshere"}
				}
			);
        }, 15);
    });
 
    bot.start(dpp::st_wait);
}
