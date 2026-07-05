#include "command_execution.h"

#include "command.h"
#include "hierarchy.h"
#include "history.h"
#include "listing.h"
#include "todo.h"
#include "todo_format.h"

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
int findProjectInsertionIndex(const Todo &todo, const std::string &project)
{
	if (project.empty()) {
		return static_cast<int>(todo.lines.size());
	}

	bool matchingProject = false;
	std::size_t lastNonEmpty = 0;
	for (std::size_t i = 0; i < todo.lines.size(); ++i) {
		if (isProjectLine(todo.lines[i])) {
			if (projectNameMatches(todo.lines[i], project)) {
				matchingProject = true;
			} else if (matchingProject) {
				return static_cast<int>(lastNonEmpty + 1);
			}
		}
		if (!isBlankLine(todo.lines[i])) {
			lastNonEmpty = i;
		}
	}

	return matchingProject
		? static_cast<int>(todo.lines.size())
		: -1;
}

void addTask(Todo &todo, const AddCommand &add)
{
	std::string taskLine = createTaskLine('-', add.task, add.tag);

	if (add.hasParent) {
		validateHierarchy(todo.lines);
		if (add.parentIndex >= todo.lines.size()) {
			throw std::runtime_error("Parent index out of bounds");
		}
		if (!isTaskLine(todo.lines[add.parentIndex])) {
			throw std::runtime_error("Parent line is not a task");
		}

		const std::size_t depth = taskDepth(todo.lines[add.parentIndex]) + 1;
		taskLine.insert(0, depth * TASK_INDENT_WIDTH, ' ');
		const std::size_t index =
			subtreeEnd(todo.lines, add.parentIndex);
		todo.addLine(static_cast<unsigned>(index), taskLine);
		todo.commit();
		todo.printLine(static_cast<unsigned>(index));
		return;
	}

	int index = findProjectInsertionIndex(todo, add.project);
	if (index == -1) {
		if (!todo.lines.empty() && !isBlankLine(todo.lines.back())) {
			todo.addLine(static_cast<unsigned>(todo.lines.size()), "");
		}
		todo.addLine(
			static_cast<unsigned>(todo.lines.size()),
			std::string(1, PROJECT_FILE_CHAR) + " " + add.project);
		index = static_cast<int>(todo.lines.size());
	}

	todo.addLine(static_cast<unsigned>(index), taskLine);
	todo.commit();
	todo.printLine(static_cast<unsigned>(index));
}

void removeTask(Todo &todo, const RemoveCommand &remove)
{
	validateHierarchy(todo.lines);
	if (remove.index >= todo.lines.size()) {
		throw std::runtime_error("Index out of bounds");
	}
	if (!isTaskLine(todo.lines[remove.index])) {
		throw std::runtime_error("Line is not a task");
	}

	const std::size_t end = subtreeEnd(todo.lines, remove.index);
	todo.printLine(remove.index);
	todo.removeLines(remove.index, static_cast<unsigned>(end));
	todo.commit();
}

void setTaskStatus(Todo &todo, uint32_t index, char status, bool tree)
{
	validateHierarchy(todo.lines);
	if (index >= todo.lines.size()) {
		throw std::runtime_error("Index out of bounds");
	}
	if (!isTaskLine(todo.lines[index])) {
		throw std::runtime_error("Line is not a task");
	}

	const std::size_t end = tree
		? subtreeEnd(todo.lines, index)
		: index + 1;
	for (std::size_t i = index; i < end; ++i) {
		if (isTaskLine(todo.lines[i])) {
			todo.setStatus(static_cast<unsigned>(i), status);
		}
	}
	todo.commit();
	todo.printLine(index);
}

void undoLastChange(Todo &todo)
{
	History history("history.txt");
	const std::vector<std::string> snapshot = history.last();
	const std::vector<std::string> linesBeforeUndo = todo.lines;

	todo.lines = snapshot;
	todo.commit();

	history.pop();
	try {
		history.commit();
	} catch (...) {
		todo.lines = linesBeforeUndo;
		todo.commit();
		throw;
	}
}

void tidyTodo(Todo &todo)
{
	validateHierarchy(todo.lines);

	for (std::size_t i = 1; i < todo.lines.size(); ++i) {
		if (isBlankLine(todo.lines[i - 1]) &&
			isBlankLine(todo.lines[i])) {
			todo.removeLine(static_cast<unsigned>(i--));
			continue;
		}
		if (isBlankLine(todo.lines[i - 1]) &&
			!isProjectLine(todo.lines[i])) {
			todo.removeLine(static_cast<unsigned>(--i));
		}
	}

	for (std::size_t i = 1; i < todo.lines.size(); ++i) {
		if (!isBlankLine(todo.lines[i - 1]) &&
			isProjectLine(todo.lines[i])) {
			todo.addLine(static_cast<unsigned>(i), "");
		}
	}

	todo.commit();
}
}

void executeCommand(Todo &todo, const Command &command)
{
	switch (command.type()) {
	case List:
		executeListCommand(todo, command);
		break;
	case Add:
		addTask(todo, command.add);
		break;
	case Remove:
		removeTask(todo, command.remove);
		break;
	case Doo:
		setTaskStatus(
			todo, command.doo.index, command.doo.status, command.doo.tree);
		break;
	case Set:
		setTaskStatus(
			todo, command.set.index, command.set.status, command.set.tree);
		break;
	case Undo:
		undoLastChange(todo);
		break;
	case Tidy:
		tidyTodo(todo);
		break;
	case None:
		break;
	}
}
