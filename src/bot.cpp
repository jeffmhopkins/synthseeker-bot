include <dpp/dpp.h>

#include "md5.h"
#include "bot.h"

long discord_channel = 0;
long discord_patron_role = 0;
long discord_jammer_role = 0;
long discord_admin_id = 0;
std::string discord_notification_role = "<@&0>";
std::string discord_token = "0";
std::string salt = "0";
std::string info_message = "info Message";

std::string alexa_on_url = "http:";
std::string alexa_off_url = "http:";


//Internal Temporary Server Variables
bool server_needs_announcement = false;
bool allow_announcements = true;
bool just_booted = true;
std::string ninjam_user_list = "";
std::string ninjam_user_list_get = "";
int get_status = 0;
MD5 md5;

bool is_alexa_enabled = false;


int main() {
	dpp::cluster bot(discord_token);
	bot.on_log(dpp::utility::cout_logger());
	bot.on_slashcommand([ & bot](const dpp::slashcommand_t & event) {
		bool is_patron = false;
		bool is_jammer = false;
		bool is_admin  = false;
		std::vector < dpp::snowflake > roles_list = event.command.member.get_roles();
		dpp::snowflake user_id = event.command.member.user_id;
		if(user_id == discord_admin_id) {
			is_patron = true;
			is_jammer = true;
			is_admin  = true;
		} else {
			for(int i = 0; i < roles_list.size(); ++i) {
				if(roles_list[i] == discord_patron_role) {
					is_patron = true;
				}
				if(roles_list[i] == discord_jammer_role) {
					is_jammer = true;
				}
			}
		}
		//Commands if you are not a patron
		if(!(is_patron || is_jammer || is_admin)) {
			event.reply(dpp::message("You do not have proper roles membership for this command. (Need to be a Patron)").set_flags(dpp::m_ephemeral));
		} else if(is_patron) {
			//Commands if you are a patron
			if(event.command.get_command_name() == "info") {
				event.reply(dpp::message(info_message).set_flags(dpp::m_ephemeral));
			} else if(event.command.get_command_name() == "website_authenticate") {
				if(is_jammer) {
					long uid = (long) user_id;
					long rid = discord_jammer_role;
					std::string message = "Click here to get a website cookie: ";
					std::string link = "https://ninjam.synthseeker.online/util/auth.php?a=auth&userid=";
					link += std::to_string(uid);
					link += "&roleid=";
					link += std::to_string(rid);
					link += "&auth=";
					link += md5(event.command.member.get_nickname()+std::to_string(uid) + std::to_string(rid) + salt);
					link += "&name=";
					link += escape(event.command.member.get_nickname());
					event.reply(dpp::message(message + link).set_flags(dpp::m_ephemeral));
				} else {
					event.reply(dpp::message("You do not have proper roles membership for this command. (Need to be a Jammer)").set_flags(dpp::m_ephemeral));
				}
			} else if(event.command.get_command_name() == "configure_announcements") {
				if(is_jammer) {
					std::string announcement_enable = std::get < std::string > (event.get_parameter("announcements"));
					if(announcement_enable == "announce_enable") {
						event.reply(dpp::message("Channel announcements are now enabled.").set_flags(dpp::m_ephemeral));
						allow_announcements = true;
					} else if(announcement_enable == "announce_disable") {
						event.reply(dpp::message("Channel announcements are now disabled.").set_flags(dpp::m_ephemeral));
						allow_announcements = false;
					}
				} else {
					event.reply(dpp::message("You do not have proper roles membership for this command. (Need to be a Jammer)").set_flags(dpp::m_ephemeral));
				}
			} else if(event.command.get_command_name() == "configure_home_automation") {
				if(is_admin) {
					std::string alexa_enable = std::get < std::string > (event.get_parameter("home_automation"));
					if(alexa_enable == "alexa_enable") {
						event.reply(dpp::message("Home automation is now enabled.").set_flags(dpp::m_ephemeral));
						is_alexa_enabled = true;
					} else if(alexa_enable == "alexa_disable") {
						event.reply(dpp::message("Home automation is now disabled.").set_flags(dpp::m_ephemeral));
						is_alexa_enabled = false;
					}
				} else {
					event.reply(dpp::message("You do not have proper roles membership for this command. (Need to be admin)").set_flags(dpp::m_ephemeral));
				}
			} else if(event.command.get_command_name() == "restart_server") {
				if(is_admin) {
					event.reply(dpp::message("Attempting to perform server restart...").set_flags(dpp::m_ephemeral));
					bot.request("http://ninjam.synthseeker.online/util/discord.php?a=restart", dpp::m_get, [ & bot](const dpp::http_request_completion_t & cc) {
						if(cc.status == 200) {
							bot.set_presence(dpp::presence(dpp::presence_status::ps_online, dpp::activity_type::at_custom, "Restarting Server..."));
						} else {
							bot.set_presence(dpp::presence(dpp::presence_status::ps_online, dpp::activity_type::at_custom, "Failed restart..."));
						}
					}, "", "application/json", {
						{
							"Authorization",
							"Bearer tokengoeshere"
						}
					});
				} else {
					event.reply(dpp::message("You do not have proper roles membership for this command. (Need to be a Ninjam Admin)").set_flags(dpp::m_ephemeral));
				}
			} else if(event.command.get_command_name() == "server_status") {
				if(is_admin) {
					bot.request("http://ninjam.synthseeker.online/util/discord.php?a=status", dpp::m_get, [ & bot, event](const dpp::http_request_completion_t & cc) {
						event.reply(dpp::message(cc.body).set_flags(dpp::m_ephemeral));
					}, "", "application/json", {
						{
							"Authorization",
							"Bearer tokengoeshere"
						}
					});
				} else {
					event.reply(dpp::message("You do not have proper roles membership for this command. (Need to be a Ninjam Admin)").set_flags(dpp::m_ephemeral));
				}
			} else {
				event.reply(dpp::message("I'm sorry, unknown command.").set_flags(dpp::m_ephemeral));
			}
		}
	});
	bot.on_ready([ & bot](const dpp::ready_t & event) {
		//if (dpp::run_once<struct clear_bot_commands>()) {
		//bot.global_bulk_command_delete();
		//}
		if(dpp::run_once < struct register_bot_commands > ()) {
			bot.global_command_create(dpp::slashcommand("info", "Information about the NINJAM Server", bot.me.id));
			bot.global_command_create(dpp::slashcommand("website_authenticate", "Get a cookie that elevates permissions on the website.", bot.me.id));
			
			dpp::slashcommand settingsannouncements("configure_announcements", "(jammer) change settings of the NINJAM bot.", bot.me.id);
			settingsannouncements.add_option(dpp::command_option(dpp::co_string, "announcements", "Allow server change messages in channel", true).add_choice(dpp::command_option_choice("enable", std::string("announce_enable"))).add_choice(dpp::command_option_choice("disable", std::string("announce_disable"))));
			
			dpp::slashcommand settingsalexa("configure_home_automation", "(admin) change home automation settings.", bot.me.id);
			settingsalexa.add_option(dpp::command_option(dpp::co_string, "home_automation", "Change the home automation settings", true).add_choice(dpp::command_option_choice("enable", std::string("alexa_enable"))).add_choice(dpp::command_option_choice("disable", std::string("alexa_disable"))));
			
			bot.global_command_create(settingsannouncements);
			bot.global_command_create(settingsalexa);
			
			bot.global_command_create(dpp::slashcommand("restart_server", "admin command to restart the server", bot.me.id));
			bot.global_command_create(dpp::slashcommand("server_status", "admin command to view stats on the server", bot.me.id));
			bot.set_presence(dpp::presence(dpp::presence_status::ps_online, dpp::activity_type::at_custom, "Server reboot success"));
		}
		bot.start_timer([ & bot](const dpp::timer & timer) {
			bot.request("http://ninjam.synthseeker.online/util/discord.php?a=users", dpp::m_get, [ & bot](const dpp::http_request_completion_t & cc) {
				ninjam_user_list_get = cc.body;
				get_status = cc.status;
				if((ninjam_user_list_get.length() > 3) && (get_status == 200)) {
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
								bot.message_create(dpp::message(discord_channel, discord_notification_role + ", Server empty. https://ninjam.synthseeker.online/recordings/"));
							}
						}
						if(is_alexa_enabled&& !just_booted) {
							bot.request(
								alexa_off_url, dpp::m_get, [](const dpp::http_request_completion_t & cc) {
									std::cout << "I got reply: " << cc.body << " with HTTP status code: " << cc.status << "\n";
								},
								"",
								"application/json",
								{
									{"Authorization", "Bearer tokengoeshere"}
								}
							);
						}
						bot.set_presence(dpp::presence(dpp::presence_status::ps_online, dpp::activity_type::at_custom, "Server empty"));
					} else {
						if(allow_announcements) {
							bot.message_create(dpp::message(discord_channel, discord_notification_role + ", " + ninjam_user_list + " online! https://ninjam.synthseeker.online/live/"));
						}
						bot.set_presence(dpp::presence(dpp::presence_status::ps_online, dpp::activity_type::at_custom, ninjam_user_list + " online!"));
						if(is_alexa_enabled) {
							bot.request(
								alexa_on_url, dpp::m_get, [](const dpp::http_request_completion_t & cc) {
									std::cout << "I got reply: " << cc.body << " with HTTP status code: " << cc.status << "\n";
								},
								"",
								"application/json",
								{
									{"Authorization", "Bearer tokengoeshere"}
								}
							);
						}
					}
					server_needs_announcement = false;
					just_booted = false;
				}
			}, "", "application/json", {
				{
					"Authorization",
					"Bearer tokengoeshere"
				}
			});
		}, 15);
	});
	bot.start(dpp::st_wait);
}

std::string escape(const std::string & text) {
	std::ostringstream escaped_text;
	for(char c: text) {
		switch(c) {
			case '&':
				escaped_text << "&amp;";
				break;
			case '<':
				escaped_text << "&lt;";
				break;
			case '>':
				escaped_text << "&gt;";
				break;
			case ' ':
				escaped_text << "%20";
				break;
			default:
				escaped_text << c;
		}
	}
	return escaped_text.str();
}
