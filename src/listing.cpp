#include "listing.h"

#include "command.h"
#include "hierarchy.h"
#include "todo.h"
#include "todo_format.h"
#include "utility.h"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <set>
#include <string>
#include <vector>

namespace {
struct NumberedLine {
	std::string text;
	uint32_t number;
};

struct ProjectBlock {
	std::string name;
	uint32_t headingNumber;
	std::string heading;
	std::vector<NumberedLine> lines;
};

std::vector<ProjectBlock> groupByProject(const Todo &todo)
{
	std::vector<ProjectBlock> projects;

	for (std::size_t i = 0; i < todo.lines.size(); ++i) {
		const std::string &line = todo.lines[i];
		if (isProjectLine(line)) {
			projects.push_back({
				projectNameFromLine(line),
				static_cast<uint32_t>(i),
				line,
				std::vector<NumberedLine>()
			});
			continue;
		}

		if (projects.empty()) {
			projects.push_back({
				std::string(), 0, std::string(),
				std::vector<NumberedLine>()
			});
		}
		projects.back().lines.push_back({
			line, static_cast<uint32_t>(i)
		});
	}

	return projects;
}

bool projectMatches(const ProjectBlock &project, const Command &command)
{
	return command.list.project.empty() ||
		equalsIgnoreCase(project.name, command.list.project);
}

bool taskMatches(const std::string &line, const Command &command)
{
	if (!isTaskLine(line)) {
		return false;
	}
	if (!command.list.tags.empty() &&
		!containsAnyTag(line, command.list.tags)) {
		return false;
	}
	if (!command.list.statuses.empty() &&
		!hasAnyStatus(line, command.list.statuses)) {
		return false;
	}
	return true;
}

void listProjects(const std::vector<ProjectBlock> &projects,
				  const Command &command)
{
	const bool filtered = !command.list.tags.empty() ||
		!command.list.statuses.empty();

	for (const ProjectBlock &project : projects) {
		if (project.heading.empty() || !projectMatches(project, command)) {
			continue;
		}
		if (filtered &&
			std::none_of(project.lines.begin(), project.lines.end(),
				[&command](const NumberedLine &line) {
					return taskMatches(line.text, command);
				})) {
			continue;
		}
		printTodoLine(project.heading, project.headingNumber);
	}
}

void listTags(const std::vector<ProjectBlock> &projects,
			  const Command &command)
{
	std::set<std::string> tags;

	for (const ProjectBlock &project : projects) {
		if (!projectMatches(project, command)) {
			continue;
		}
		for (const NumberedLine &line : project.lines) {
			if (!isTaskLine(line.text)) {
				continue;
			}
			if (!command.list.statuses.empty() &&
				!hasAnyStatus(line.text, command.list.statuses)) {
				continue;
			}

			for (const std::string &tag : tagsFromLine(line.text)) {
				if (command.list.tags.empty() ||
					containsAnyTag(
						std::string(1, TAG_CHAR) + tag,
						command.list.tags)) {
					tags.insert(tag);
				}
			}
		}
	}

	for (const std::string &tag : tags) {
		std::cout << "  " << TAG_CHAR << tag << '\n';
	}
}

void listTasks(const Todo &todo,
			   const std::vector<ProjectBlock> &projects,
			   const Command &command)
{
	const bool filtered = !command.list.tags.empty() ||
		!command.list.statuses.empty();
	if (filtered) {
		validateHierarchy(todo.lines);
	}

	for (const ProjectBlock &project : projects) {
		if (!projectMatches(project, command)) {
			continue;
		}

		std::set<uint32_t> visibleLines;
		if (filtered) {
			for (const NumberedLine &line : project.lines) {
				if (!taskMatches(line.text, command)) {
					continue;
				}

				visibleLines.insert(line.number);
				for (std::size_t ancestor :
					ancestorTaskIndices(todo.lines, line.number)) {
					visibleLines.insert(static_cast<uint32_t>(ancestor));
				}
			}
			if (visibleLines.empty()) {
				continue;
			}
		}

		if (!project.heading.empty()) {
			printTodoLine(project.heading, project.headingNumber);
		}
		for (const NumberedLine &line : project.lines) {
			if (!filtered ||
				visibleLines.find(line.number) != visibleLines.end()) {
				printTodoLine(line.text, line.number);
			}
		}
	}
}
}

void executeListCommand(const Todo &todo, const Command &command)
{
	const std::vector<ProjectBlock> projects = groupByProject(todo);

	switch (command.list.mode) {
	case ListMode::Projects:
		listProjects(projects, command);
		break;
	case ListMode::Tags:
		listTags(projects, command);
		break;
	case ListMode::Tasks:
		listTasks(todo, projects, command);
		break;
	}
}
